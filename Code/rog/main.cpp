
#include "rog_colors.hpp"
#include "rog_direction.hpp"
#include "rog_entity.hpp"
#include "rog_feature.hpp"
#include "rog_level_gen.hpp"
#include "rog_player.hpp"
#include "rog_screen_drawing.hpp"
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

	namespace actions
	{

		struct move { direction m_dir; };
		struct use_stairs { stairs_direction m_dir; };

	} // actions

	using action = std::variant
	<
		actions::move,
		actions::use_stairs
	>;

	auto constexpr MAX_QUEUED_ACTIONS = std::size_t{ 5 };

	auto constexpr TIME_PER_CYCLE = std::chrono::duration_cast<bump::high_res_duration_t>(std::chrono::duration<float>(0.05f));
	auto constexpr TIME_PER_TURN = TIME_PER_CYCLE * 10;

	bump::gamestate main_loop(bump::app& app, std::int32_t level_depth)
	{
		bump::log_info("main loop - start");

		// setup
		auto const tile_size = glm::ivec2{ 24, 36 };
		auto tile_renderer = rog::tile_renderer(app, glm::vec2(tile_size));

		auto screen_buffer = bump::grid2<screen::cell>();
		screen::resize(screen_buffer, app.m_window.get_size(), tile_size, { '#', colors::light_red, colors::dark_red });

		auto app_events   = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();

		// main loop
		auto paused = false;
		
		auto level = level_gen::generate_level(level_depth);
		auto player = player_create_entity(level.m_registry);
		auto player_handle = entt::handle{ level.m_registry, player };

		auto action_queue = std::deque<action>();

		auto timer = bump::frame_timer(bump::high_res_duration_t{ 0 });
		auto time_accumulator = bump::high_res_duration_t{ 0 };

		while (true)
		{
			// input
			{
				app.m_input_handler.poll(app_events, input_events);

				// process app events:
				while (!app_events.empty())
				{
					auto event = std::move(app_events.front());
					app_events.pop();

					namespace ae = bump::input::app_events;

					if (std::holds_alternative<ae::quit>(event))
						return { }; // todo: save!

					if (std::holds_alternative<ae::pause>(event))
					{
						auto const& p = std::get<ae::pause>(event);

						// todo: we probably want to recognise the actual event and do
						// some things (mute audio, not send input to game thread, not 
						// do rendering, etc.)
						paused = p.m_pause;

						continue;
					}

					if (std::holds_alternative<ae::resize>(event))
					{
						auto const& r = std::get<ae::resize>(event);
						auto const& window_size = r.m_size;

						screen::resize(screen_buffer, window_size, tile_size, 
							{ ' ', glm::vec3(1.0), glm::vec3(1.0, 0.0, 1.0) });

						continue;
					}
				}

				// process input events:
				while (!input_events.empty())
				{
					auto event = std::move(input_events.front());
					input_events.pop();

					namespace ie = bump::input::input_events;

					if (std::holds_alternative<ie::keyboard_key>(event))
					{
						auto const& k = std::get<ie::keyboard_key>(event);

						using kt = bump::input::keyboard_key;

						// app inputs
						if (k.m_key == kt::ESCAPE && k.m_value)
							return { }; // todo: save!

						// action inputs
						if (action_queue.size() < MAX_QUEUED_ACTIONS)
						{
							     if (k.m_key == kt::NUM7 && k.m_value) { if (player_can_move(player_handle, level, direction::LEFT_UP)) action_queue.push_back(actions::move{ direction::LEFT_UP }); }
							else if (k.m_key == kt::NUM8 && k.m_value) { if (player_can_move(player_handle, level, direction::UP)) action_queue.push_back(actions::move{ direction::UP }); }
							else if (k.m_key == kt::NUM9 && k.m_value) { if (player_can_move(player_handle, level, direction::RIGHT_UP)) action_queue.push_back(actions::move{ direction::RIGHT_UP }); }
							else if (k.m_key == kt::NUM4 && k.m_value) { if (player_can_move(player_handle, level, direction::LEFT)) action_queue.push_back(actions::move{ direction::LEFT }); }
							else if (k.m_key == kt::NUM6 && k.m_value) { if (player_can_move(player_handle, level, direction::RIGHT)) action_queue.push_back(actions::move{ direction::RIGHT }); }
							else if (k.m_key == kt::NUM1 && k.m_value) { if (player_can_move(player_handle, level, direction::LEFT_DOWN)) action_queue.push_back(actions::move{ direction::LEFT_DOWN }); }
							else if (k.m_key == kt::NUM2 && k.m_value) { if (player_can_move(player_handle, level, direction::DOWN)) action_queue.push_back(actions::move{ direction::DOWN }); }
							else if (k.m_key == kt::NUM3 && k.m_value) { if (player_can_move(player_handle, level, direction::RIGHT_DOWN)) action_queue.push_back(actions::move{ direction::RIGHT_DOWN }); }

							else if (k.m_key == kt::DOT && k.m_value && k.m_mods.shift() &&
							         player_can_use_stairs(player_handle, level, stairs_direction::DOWN))
							{
								action_queue.push_back(actions::use_stairs{ stairs_direction::DOWN });
							}
							else if (k.m_key == kt::COMMA && k.m_value && k.m_mods.shift() &&
							         player_can_use_stairs(player_handle, level, stairs_direction::UP))
							{
								action_queue.push_back(actions::use_stairs{ stairs_direction::UP });
							}
						}
						
						continue;
					}
				}
			}

			// update
			{
				time_accumulator += timer.get_last_frame_time();

				// update:
				if (time_accumulator >= TIME_PER_CYCLE)
				{
					time_accumulator -= TIME_PER_CYCLE;

					actors_add_energy(level.m_registry);

					if (actor_has_turn_energy({ level.m_registry, player }))
					{
						bump::log_info("player turn!");

						// do player action!
						if (!action_queue.empty())
						{
							auto action = std::move(action_queue.front());
							action_queue.pop_front();

							if (std::holds_alternative<actions::move>(action))
							{
								auto const& move = std::get<actions::move>(action);
								player_move(player_handle, level, move.m_dir);
							}
							else if (std::holds_alternative<actions::use_stairs>(action))
							{
								auto const& use_stairs = std::get<actions::use_stairs>(action);
								auto const delta_depth = (use_stairs.m_dir == stairs_direction::UP ? +1 : -1);
								auto const next_depth = level_depth + delta_depth;
								return { [&, next_depth] (bump::app& app) { return main_loop(app, next_depth); } };
							}
						}

						actor_take_turn_energy({ level.m_registry, player });
					}

					// todo: other actor turns!

					// validate action queue
					{
						auto a = action_queue.begin();

						for ( ; a != action_queue.end(); ++a)
						{
							if (std::holds_alternative<actions::move>(*a))
							{
								auto const& move = std::get<actions::move>(*a);
								if (!player_can_move(player_handle, level, move.m_dir))
									break;
								
								continue;
							}

							if (std::holds_alternative<actions::use_stairs>(*a))
							{
								auto const& use_stairs = std::get<actions::use_stairs>(*a);
								if (player_can_use_stairs(player_handle, level, use_stairs.m_dir))
									break;
								
								continue;
							}

							bump::die(); // should be unreachable
						}

						action_queue.erase(a, action_queue.end());
					}
				}
			}
			
			// drawing - todo: not every frame?
			{
				auto const& player_pos = level.m_registry.get<comp_position>(player);
				auto const& player_vis = level.m_registry.get<comp_visual>(player);

				screen::fill(screen_buffer, { ' ', colors::black, colors::black });
				screen::draw(screen_buffer, level.m_grid, player_pos.m_pos, player_vis.m_cell);
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
	
		bump::log_info("main loop - exit");
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
		bump::run_state({ [] (bump::app& app) { return rog::main_loop(app, 1); } }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}
