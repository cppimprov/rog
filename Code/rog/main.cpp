
#include "rog_actor.hpp"
#include "rog_colors.hpp"
#include "rog_direction.hpp"
#include "rog_entity.hpp"
#include "rog_feature.hpp"
#include "rog_level_gen.hpp"
#include "rog_monster.hpp"
#include "rog_player.hpp"
#include "rog_player_action.hpp"
#include "rog_random.hpp"
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

	auto constexpr TIME_PER_CYCLE = bump::high_res_duration_from_seconds(0.05f);
	auto constexpr TIME_PER_TURN = TIME_PER_CYCLE * 10;

	bump::gamestate main_loop(bump::app& app, std::int32_t level_depth)
	{
		bump::log_info("main loop - start");

		// setup
		auto rng = random::seed_rng();

		auto const tile_size = glm::ivec2{ 24, 36 };
		auto tile_renderer = rog::tile_renderer(app, glm::vec2(tile_size));

		auto screen_buffer = bump::grid2<screen::cell>();
		screen::resize(screen_buffer, app.m_window.get_size(), tile_size, { '#', colors::light_red, colors::dark_red });

		auto app_events   = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();

		// main loop
		auto app_paused = false;
		auto player_paused = false;
		
		auto level = level_gen::generate_level(level_depth, rng);

		auto queued_action = std::optional<player_action>();

		auto timer = bump::frame_timer(bump::high_res_duration_t{ 0 });
		auto time_accumulator = bump::high_res_duration_t{ 0 };

		auto hovered_tile = std::optional<glm::size2>();
		auto queued_path = std::vector<glm::size2>();

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
						app_paused = p.m_pause; // todo: mute audio, etc.

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
						
						if (k.m_key == kt::SPACE && k.m_value)
							player_paused = !player_paused;

						// action inputs
						else if (k.m_key == kt::NUM7 && k.m_value) queued_action = player_actions::move{ direction::LEFT_UP };
						else if (k.m_key == kt::NUM8 && k.m_value) queued_action = player_actions::move{ direction::UP };
						else if (k.m_key == kt::NUM9 && k.m_value) queued_action = player_actions::move{ direction::RIGHT_UP };
						else if (k.m_key == kt::NUM4 && k.m_value) queued_action = player_actions::move{ direction::LEFT };
						else if (k.m_key == kt::NUM6 && k.m_value) queued_action = player_actions::move{ direction::RIGHT };
						else if (k.m_key == kt::NUM1 && k.m_value) queued_action = player_actions::move{ direction::LEFT_DOWN };
						else if (k.m_key == kt::NUM2 && k.m_value) queued_action = player_actions::move{ direction::DOWN };
						else if (k.m_key == kt::NUM3 && k.m_value) queued_action = player_actions::move{ direction::RIGHT_DOWN };
						else if (k.m_key == kt::DOT && k.m_value && k.m_mods.shift())   queued_action = player_actions::use_stairs{ stairs_direction::DOWN };
						else if (k.m_key == kt::COMMA && k.m_value && k.m_mods.shift()) queued_action = player_actions::use_stairs{ stairs_direction::UP };

						if (queued_action.has_value())
							queued_path.clear();

						continue;
					}

					if (std::holds_alternative<ie::mouse_motion>(event))
					{
						auto const& m = std::get<ie::mouse_motion>(event);

						auto const player_pos = level.m_registry.get<comp_position>(level.m_player).m_pos;
						auto const origin = screen::get_panel_origin(level.m_grid.extents(), screen_buffer.extents(), player_pos);

						auto const mouse_pos_px = glm::ivec2{ m.m_position.x, (app.m_window.get_size().y - 1) - m.m_position.y };
						auto const mouse_pos_tiles = mouse_pos_px / tile_size;

						auto const level_size = level.m_grid.extents();
						auto const tile = origin + glm::size2(mouse_pos_tiles);

						hovered_tile = (tile.x < level_size.x && tile.y < level_size.y) ? std::optional<glm::size2>(tile) : std::optional<glm::size2>(std::nullopt);

						continue;
					}

					if (std::holds_alternative<ie::mouse_button>(event))
					{
						auto const& m = std::get<ie::mouse_button>(event);

						using bt = bump::input::mouse_button;

						if (m.m_button != bt::LEFT)
							continue;
						
						auto const player_pos = level.m_registry.get<comp_position>(level.m_player).m_pos;
						auto const origin = screen::get_panel_origin(level.m_grid.extents(), screen_buffer.extents(), player_pos);

						auto const mouse_pos_px = glm::ivec2{ m.m_position.x, (app.m_window.get_size().y - 1) - m.m_position.y };
						auto const mouse_pos_tiles = mouse_pos_px / tile_size;

						auto const level_size = level.m_grid.extents();
						auto const tile = origin + glm::size2(mouse_pos_tiles);

						if (tile.x < level_size.x && tile.y < level_size.y)
						{
							queued_path = find_path(level.m_grid, player_pos, tile);
							queued_action.reset();
						}

						continue;
					}
				}
			}

			// update
			{	
				if (app_paused || player_paused)
					time_accumulator = bump::high_res_duration_t{ 0 };
				else
					time_accumulator += timer.get_last_frame_time();

				// do cycle
				if (time_accumulator >= TIME_PER_CYCLE)
				{
					time_accumulator -= TIME_PER_CYCLE;

					bump::log_info("cycle");

					// add cycle energy
					{
						auto view = level.m_registry.view<comp_actor>();

						for (auto a : view)
							actor_add_energy(view.get<comp_actor>(a));
					}

					// player turn
					if (actor_has_turn_energy(level.m_registry.get<comp_actor>(level.m_player)))
					{
						bump::log_info("player turn!");

						// do player action!
						if (queued_action.has_value())
						{
							auto action = std::move(queued_action.value());
							queued_action.reset();

							namespace pa = player_actions;

							if (std::holds_alternative<pa::move>(action))
							{
								auto const& move = std::get<pa::move>(action);

								auto& pos = level.m_registry.get<comp_position>(level.m_player);
								if (!move_actor(level, level.m_player, pos, move.m_dir))
								{
									bump::log_info("There is something in the way.");
								}
							}
							else if (std::holds_alternative<pa::use_stairs>(action))
							{
								auto const& use_stairs = std::get<pa::use_stairs>(action);

								if (player_can_use_stairs(level, use_stairs.m_dir))
								{
									auto const delta_depth = (use_stairs.m_dir == stairs_direction::UP ? +1 : -1);
									auto const next_depth = level_depth + delta_depth;
									return { [&, next_depth] (bump::app& app) { return main_loop(app, next_depth); } };
								}
							}
						}
						else if (!queued_path.empty())
						{
							auto& pos = level.m_registry.get<comp_position>(level.m_player);

							auto target = queued_path.back();
							queued_path.pop_back();

							if (!move_actor(level, level.m_player, pos, target))
							{
								bump::log_info("There is something in the way.");
								queued_path.clear();
							}
						}

						actor_take_turn_energy(level.m_registry.get<comp_actor>(level.m_player));
					}

					// monster turn
					{
						auto view = level.m_registry.view<comp_actor, comp_monster_tag, comp_position>();

						for (auto m : view)
						{
							auto [p, a] = view.get<comp_position, comp_actor>(m);

							if (!actor_has_turn_energy(a))
								continue;
							
							bump::log_info("monster turn!");
							
							monster_move(level, m, p, rng);

							actor_take_turn_energy(a);
						}
					}
				}
			}
			
			// drawing - todo: only if something changes?
			{
				screen::fill(screen_buffer, { ' ', colors::black, colors::black });
				screen::draw(screen_buffer, level, queued_path, hovered_tile);
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
