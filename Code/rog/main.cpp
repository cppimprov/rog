
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

	bool actor_can_take_turn(entt::handle actor_handle)
	{
		return actor_handle.get<comp_actor>().m_energy >= ACTOR_ENERGY_PER_TURN;
	}

	bump::gamestate play_level(
		bump::app& app, 
		thread_switch& ts,
		std::queue<bump::input::input_event>& input_events, 
		bump::grid2<screen::cell>& screen_buffer, 
		bool const& request_quit,
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

				auto player_turn_complete = false;

				while (!player_turn_complete)
				{
					ts.notify_main_thread_and_wait();

					if (request_quit)
						return { }; // todo: save game.

					auto change_level = std::optional<stairs_direction>();
					while (!input_events.empty())
					{
						auto event = std::move(input_events.front());
						input_events.pop();

						if (std::holds_alternative<input::input_events::keyboard_key>(event))
						{
							auto const& key = std::get<input::input_events::keyboard_key>(event);

							     if (key.m_key == input::keyboard_key::NUM7 && key.m_value) { player_move({ level.m_registry, player }, level, direction::LEFT_UP); player_turn_complete = true; break; }
							else if (key.m_key == input::keyboard_key::NUM8 && key.m_value) { player_move({ level.m_registry, player }, level, direction::UP); player_turn_complete = true; break; }
							else if (key.m_key == input::keyboard_key::NUM9 && key.m_value) { player_move({ level.m_registry, player }, level, direction::RIGHT_UP); player_turn_complete = true; break; }
							else if (key.m_key == input::keyboard_key::NUM4 && key.m_value) { player_move({ level.m_registry, player }, level, direction::LEFT); player_turn_complete = true; break; }
							else if (key.m_key == input::keyboard_key::NUM6 && key.m_value) { player_move({ level.m_registry, player }, level, direction::RIGHT); player_turn_complete = true; break; }
							else if (key.m_key == input::keyboard_key::NUM1 && key.m_value) { player_move({ level.m_registry, player }, level, direction::LEFT_DOWN); player_turn_complete = true; break; }
							else if (key.m_key == input::keyboard_key::NUM2 && key.m_value) { player_move({ level.m_registry, player }, level, direction::DOWN); player_turn_complete = true; break; }
							else if (key.m_key == input::keyboard_key::NUM3 && key.m_value) { player_move({ level.m_registry, player }, level, direction::RIGHT_DOWN); player_turn_complete = true; break; }

							else if (key.m_key == input::keyboard_key::DOT && key.m_value && 
							         app.m_input_handler.is_keyboard_key_pressed(bump::input::keyboard_key::LEFTSHIFT) && 
							         player_use_stairs({ level.m_registry, player }, level, stairs_direction::DOWN))
							{
								change_level = stairs_direction::DOWN;
								player_turn_complete = true;
								break;
							}
							else if (key.m_key == input::keyboard_key::COMMA && key.m_value && 
							         app.m_input_handler.is_keyboard_key_pressed(bump::input::keyboard_key::LEFTSHIFT) && 
							         player_use_stairs({ level.m_registry, player }, level, stairs_direction::UP))
							{
								change_level = stairs_direction::UP;
								player_turn_complete = true;
								break;
							}
							else if (key.m_key == input::keyboard_key::ESCAPE && key.m_value)
								return { };

							continue;
						}
					}
					
					if (change_level)
					{
						auto const next_depth = level_depth + (change_level == stairs_direction::UP ? 1 : -1);
						return { [&, next_depth] (bump::app& app) { return play_level(app, ts, input_events, screen_buffer, request_quit, next_depth); } };
					}
				}

				// todo: other actor turns!
			}
		}

		return { };
	}

	void play_game(
		bump::app& app, 
		thread_switch& ts,
		std::queue<bump::input::input_event>& input_events, 
		bump::grid2<screen::cell>& screen_buffer, 
		bool const& request_quit,
		bool& game_thread_done)
	{
		using namespace bump;

		auto state_wrapper = bump::gamestate{ [&] (bump::app& app) { return play_level(app, ts, input_events, screen_buffer, request_quit, 1); } };
		bump::run_state(state_wrapper, app);

		game_thread_done = true;
		ts.notify_main_thread();
	}

	void main_loop(bump::app& app)
	{
		using namespace bump;

		log_info("main loop - start");

		auto const tile_size = glm::ivec2{ 24, 36 };
		auto tile_renderer = rog::tile_renderer(app, glm::vec2(tile_size));

		auto screen_buffer = grid2<screen::cell>();
		screen::resize(screen_buffer, app.m_window.get_size(), tile_size, { '#', colors::light_red, colors::dark_red });

		auto request_quit = false;
		auto game_thread_done = false;
		auto paused = false;
		auto input_events = std::queue<input::input_event>();
		auto app_events = std::queue<input::app_event>();
		
		auto timer = frame_timer();

		{
			auto ts = thread_switch();
			auto game_thread = std::thread(play_game, std::ref(app), std::ref(ts), std::ref(input_events), std::ref(screen_buffer), std::cref(request_quit), std::ref(game_thread_done));
			ts.notify_game_thread_and_wait();

			while (true)
			{
				// input
				{
					app.m_input_handler.poll(input_events, app_events);

					// process app events:
					while (!app_events.empty())
					{
						auto event = std::move(app_events.front());
						app_events.pop();

						if (std::holds_alternative<input::app_events::quit>(event))
						{
							request_quit = true;

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
				ts.notify_game_thread_and_wait();
				
				if (game_thread_done)
					break;

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
		}

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
