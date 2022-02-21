
#include "rog_colors.hpp"
#include "rog_direction.hpp"
#include "rog_entity.hpp"
#include "rog_feature.hpp"
#include "rog_level_gen.hpp"
#include "rog_player.hpp"
#include "rog_screen_drawing.hpp"
#include "rog_thread_switch.hpp"
#include "rog_tile_renderer.hpp"

#include <bump_app.hpp>
#include <bump_assets.hpp>
#include <bump_camera.hpp>
#include <bump_die.hpp>
#include <bump_gamestate.hpp>
#include <bump_gl.hpp>
#include <bump_grid.hpp>
#include <bump_input.hpp>
#include <bump_log.hpp>
#include <bump_math.hpp>
#include <bump_timer.hpp>
#include <bump_transform.hpp>
#include <bump_range.hpp>
#include <bump_render_text.hpp>

#include <SDL.h>
#include <SDL_main.h>

#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <thread>

namespace rog
{

	constexpr auto ACTOR_ENERGY_PER_CYCLE = std::int32_t{ 10 };
	constexpr auto ACTOR_ENERGY_PER_TURN = std::int32_t{ 100 };

	void actors_add_energy(entt::registry& registry)
	{
		auto view = registry.view<comp_actor>();

		for (auto& actor : view)
			view.get<comp_actor>(actor).m_energy += ACTOR_ENERGY_PER_CYCLE;
	}

	bool actor_has_turn_energy(entt::handle actor_handle)
	{
		return actor_handle.get<comp_actor>().m_energy >= ACTOR_ENERGY_PER_TURN;
	}

	void actor_take_turn_energy(entt::handle actor_handle)
	{
		actor_handle.get<comp_actor>().m_energy -= ACTOR_ENERGY_PER_TURN;
	}

	struct thread_context
	{
		thread_switch m_switch;
		std::queue<bump::input::input_event> m_events;
		bool m_main_thread_request_quit = false;
		bool m_game_thread_done = false;
	};

	void player_update(entt::handle player_handle, bump::app& app, thread_context& tc, level const& level, bool& quit, std::optional<stairs_direction>& change_level)
	{
		while (true)
		{
			tc.m_switch.notify_main_thread_and_wait();

			if (tc.m_main_thread_request_quit)
				return;

			while (!tc.m_events.empty())
			{
				auto event = std::move(tc.m_events.front());
				tc.m_events.pop();

				namespace ie = bump::input::input_events;
				using kt = bump::input::keyboard_key;

				if (std::holds_alternative<ie::keyboard_key>(event))
				{
					auto const& key = std::get<ie::keyboard_key>(event);

					     if (key.m_key == kt::NUM7 && key.m_value) { player_move(player_handle, level, direction::LEFT_UP); return; }
					else if (key.m_key == kt::NUM8 && key.m_value) { player_move(player_handle, level, direction::UP); return; }
					else if (key.m_key == kt::NUM9 && key.m_value) { player_move(player_handle, level, direction::RIGHT_UP); return; }
					else if (key.m_key == kt::NUM4 && key.m_value) { player_move(player_handle, level, direction::LEFT); return; }
					else if (key.m_key == kt::NUM6 && key.m_value) { player_move(player_handle, level, direction::RIGHT); return; }
					else if (key.m_key == kt::NUM1 && key.m_value) { player_move(player_handle, level, direction::LEFT_DOWN); return; }
					else if (key.m_key == kt::NUM2 && key.m_value) { player_move(player_handle, level, direction::DOWN); return; }
					else if (key.m_key == kt::NUM3 && key.m_value) { player_move(player_handle, level, direction::RIGHT_DOWN); return; }

					else if (key.m_key == kt::DOT && key.m_value && 
					         app.m_input_handler.is_keyboard_key_pressed(kt::LEFTSHIFT) && 
					         player_use_stairs(player_handle, level, stairs_direction::DOWN))
					{
						change_level = stairs_direction::DOWN;
						return;
					}
					else if (key.m_key == kt::COMMA && key.m_value && 
					         app.m_input_handler.is_keyboard_key_pressed(kt::LEFTSHIFT) && 
					         player_use_stairs(player_handle, level, stairs_direction::UP))
					{
						change_level = stairs_direction::UP;
						return;
					}
					else if (key.m_key == kt::ESCAPE && key.m_value)
					{
						quit = true;
						return;
					}

					continue;
				}
			}
		}
	}

	bump::gamestate play_level(
		bump::app& app, 
		thread_context& tc,
		bump::grid2<screen::cell>& screen_buffer, 
		std::int32_t level_depth)
	{
		using namespace bump;

		auto level = level_gen::generate_level(level_depth);
		auto player = player_create_entity(level.m_registry);
		
		while (true)
		{
			// drawing:
			{
				auto const& player_pos = level.m_registry.get<comp_position>(player);
				auto const& player_vis = level.m_registry.get<comp_visual>(player);

				screen::fill(screen_buffer, { ' ', colors::black, colors::black });
				screen::draw(screen_buffer, level.m_grid, player_pos.m_pos, player_vis.m_cell);
			}

			// update:
			{
				actors_add_energy(level.m_registry);

				if (actor_has_turn_energy({ level.m_registry, player }))
				{
					bool quit = false;
					auto change_level = std::optional<stairs_direction>();

					player_update({ level.m_registry, player }, app, tc, level, quit, change_level);

					if (tc.m_main_thread_request_quit || quit)
						return { }; // todo: save game etc.
					
					if (change_level)
					{
						auto const next_depth = level_depth + (change_level == stairs_direction::UP ? 1 : -1);
						return { [&, next_depth] (bump::app& app) { return play_level(app, tc, screen_buffer, next_depth); } };
					}

					actor_take_turn_energy({ level.m_registry, player });
				}

				// todo: other actor turns!
			}
		}

		return { };
	}

	void play_game(bump::app& app, thread_context& tc, bump::grid2<screen::cell>& screen_buffer)
	{
		using namespace bump;

		auto state_wrapper = bump::gamestate{ [&] (bump::app& app) { return play_level(app, tc, screen_buffer, 1); } };
		bump::run_state(state_wrapper, app);

		tc.m_game_thread_done = true;
		tc.m_switch.notify_main_thread();
	}

	void main_loop(bump::app& app)
	{
		using namespace bump;

		log_info("main loop - start");

		// setup
		auto const tile_size = glm::ivec2{ 24, 36 };
		auto tile_renderer = rog::tile_renderer(app, glm::vec2(tile_size));

		auto screen_buffer = grid2<screen::cell>();
		screen::resize(screen_buffer, app.m_window.get_size(), tile_size, { '#', colors::light_red, colors::dark_red });

		// start game thread
		auto tc = thread_context();
		auto app_events = std::queue<input::app_event>();
		auto game_thread = std::thread(play_game, std::ref(app), std::ref(tc), std::ref(screen_buffer));
		tc.m_switch.notify_game_thread_and_wait();

		// main loop
		auto paused = false;
		auto timer = frame_timer();

		while (true)
		{
			// input
			{
				app.m_input_handler.poll(tc.m_events, app_events);

				// process app events:
				while (!app_events.empty())
				{
					auto event = std::move(app_events.front());
					app_events.pop();

					if (std::holds_alternative<input::app_events::quit>(event))
					{
						tc.m_main_thread_request_quit = true;

						break;
					}

					if (std::holds_alternative<input::app_events::pause>(event))
					{
						auto const& p = std::get<input::app_events::pause>(event);

						// todo: we probably want to recognise the actual event and do
						// some things (mute audio, not send input to game thread, not 
						// do rendering, etc.)
						paused = p.m_pause;

						continue;
					}

					if (std::holds_alternative<input::app_events::resize>(event))
					{
						auto const& r = std::get<input::app_events::resize>(event);
						auto const& window_size = r.m_size;

						screen::resize(screen_buffer, window_size, tile_size, 
							{ ' ', glm::vec3(1.0), glm::vec3(1.0, 0.0, 1.0) });

						continue;
					}
				}

			}

			// update
			{
				tc.m_switch.notify_game_thread_and_wait();
				
				if (tc.m_game_thread_done)
					break;
			}

			// render
			{
				auto const& window_size = app.m_window.get_size();
				auto const window_size_f = glm::vec2(window_size);
				auto const window_size_u = glm::uvec2(window_size);

				auto& renderer = app.m_renderer;

				renderer.clear_color_buffers({ 0.f, 0.f, 0.f, 1.f });
				renderer.clear_depth_buffers();
				renderer.set_viewport({ 0, 0 }, window_size_u);

				tile_renderer.render(renderer, window_size_f, screen_buffer);

				app.m_window.swap_buffers();
			}

			timer.tick();
		}
	
		game_thread.join();

		log_info("main loop - exit");
	}

} // rog

int main(int , char* [])
{

	{
		auto const metadata = bump::asset_metadata
		{
			// fonts
			{
				// { "tiles", "RobotoMono-SemiBold.ttf", 30 },
			},
			// sounds
			{
				// { "intro", "intro.wav" },
			},
			// music
			{
				// { "intro", "intro.wav" },
			},
			// shaders
			{
				{ "tile", { "tile.vert", "tile.frag" } },
			},
			// models
			{
				// { "skybox", "skybox.mbp_model" },
			},
			// 2d array textures
			{
				{ "ascii_tiles", "ascii_tiles.png", 256, { GL_R8, GL_RED } },
			},
			// cubemaps
			{
				// { "skybox", { "skybox_x_pos.png", "skybox_x_neg.png", "skybox_y_pos.png", "skybox_y_neg.png", "skybox_z_pos.png", "skybox_z_neg.png" }, { GL_SRGB, GL_RGB, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR } },
			},
		};

		auto app = bump::app(metadata, { 1024, 768 }, "rog", bump::sdl::window::display_mode::WINDOWED);
		rog::main_loop(app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}
