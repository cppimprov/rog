#include "rog_gamestates.hpp"

#include "rog_ecs.hpp"
#include "rog_level.hpp"
#include "rog_level_gen.hpp"
#include "rog_player_action.hpp"
#include "rog_random.hpp"
#include "rog_screen.hpp"
#include "rog_ui.hpp"

#include <bump_app.hpp>
#include <bump_input.hpp>
#include <bump_log.hpp>
#include <bump_math.hpp>
#include <bump_timer.hpp>

namespace rog
{

	auto constexpr TIME_PER_CYCLE = bump::high_res_duration_from_seconds(0.05f);
	auto constexpr TIME_PER_TURN = TIME_PER_CYCLE * 10;

	bump::gamestate gamestate_dungeon(bump::app& app, std::int32_t level_depth)
	{
		bump::log_info("main loop - start");

		// setup
		auto rng = random::seed_rng();

		auto const tile_size_px = glm::ivec2{ 24, 36 };

		auto screen = rog::screen(
			app.m_assets.m_shaders.at("tile"),
			app.m_assets.m_textures_2d_array.at("ascii_tiles"),
			app.m_assets.m_shaders.at("tile_border"),
			app.m_window.get_size(),
			tile_size_px);

		auto ui_main = calc_ui_layout_main(screen.size());

		auto app_events   = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();

		// main loop
		auto app_paused = false;
		auto player_paused = false;
		
		auto level = level_gen::generate_level(level_depth, rng);

		auto queued_action = std::optional<player_action>();

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
						app_paused = p.m_pause; // todo: mute audio, etc.
						continue;
					}

					if (std::holds_alternative<ae::resize>(event))
					{
						auto const& r = std::get<ae::resize>(event);
						auto const& window_size = r.m_size;
						screen.resize(window_size, screen.tile_size());
						ui_main = calc_ui_layout_main(screen.size());
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
						else if (k.m_key == kt::NUM7 && k.m_value) queued_action = player_actions::move{ direction::UP_LEFT };
						else if (k.m_key == kt::NUM8 && k.m_value) queued_action = player_actions::move{ direction::UP };
						else if (k.m_key == kt::NUM9 && k.m_value) queued_action = player_actions::move{ direction::UP_RIGHT };
						else if (k.m_key == kt::NUM4 && k.m_value) queued_action = player_actions::move{ direction::LEFT };
						else if (k.m_key == kt::NUM6 && k.m_value) queued_action = player_actions::move{ direction::RIGHT };
						else if (k.m_key == kt::NUM1 && k.m_value) queued_action = player_actions::move{ direction::DOWN_LEFT };
						else if (k.m_key == kt::NUM2 && k.m_value) queued_action = player_actions::move{ direction::DOWN };
						else if (k.m_key == kt::NUM3 && k.m_value) queued_action = player_actions::move{ direction::DOWN_RIGHT };
						else if (k.m_key == kt::DOT && k.m_value && k.m_mods.shift())   queued_action = player_actions::use_stairs{ stairs_direction::DOWN };
						else if (k.m_key == kt::COMMA && k.m_value && k.m_mods.shift()) queued_action = player_actions::use_stairs{ stairs_direction::UP };

						if (queued_action.has_value())
							level.m_queued_path.clear();

						continue;
					}

					if (std::holds_alternative<ie::mouse_motion>(event))
					{
						auto const& m = std::get<ie::mouse_motion>(event);

						auto const map_panel_lv = level.get_map_panel(ui_main.m_map_sb.m_size);
						auto const mouse_pos_pn = screen.sb_to_pn(screen.px_to_sb(m.m_position), ui_main.m_map_sb.m_origin);
						auto const mouse_pos_lv = panel_cell_to_map_coords(mouse_pos_pn, map_panel_lv.m_origin);

						level.m_hovered_tile = level.in_bounds(mouse_pos_lv) ? std::optional<glm::ivec2>(mouse_pos_lv) : std::optional<glm::ivec2>();

						continue;
					}

					if (std::holds_alternative<ie::mouse_button>(event))
					{
						auto const& m = std::get<ie::mouse_button>(event);

						using bt = bump::input::mouse_button;

						if (m.m_button != bt::LEFT)
							continue;
						
						auto const map_panel_lv = level.get_map_panel(ui_main.m_map_sb.m_size);
						auto const mouse_pos_pn = screen.sb_to_pn(screen.px_to_sb(m.m_position), ui_main.m_map_sb.m_origin);
						auto const mouse_pos_lv = panel_cell_to_map_coords(mouse_pos_pn, map_panel_lv.m_origin);

						if (level.in_bounds(mouse_pos_lv))
						{
							auto const& player_pos_lv = level.m_registry.get<c_position>(level.m_player).m_pos;
							level.m_queued_path = find_path(level.m_grid, player_pos_lv, mouse_pos_lv);
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

					// add actor cycle energy
					{
						auto view = level.m_registry.view<c_actor>();

						for (auto a : view)
							view.get<c_actor>(a).add_energy();
					}

					// player turn
					if (level.m_registry.get<c_actor>(level.m_player).has_turn_energy())
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

								auto& pos = level.m_registry.get<c_position>(level.m_player);
								if (!level.move_actor(level.m_player, pos, move.m_dir))
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
									return { [&, next_depth] (bump::app& app) { return gamestate_dungeon(app, next_depth); } };
								}
							}
						}
						else if (!level.m_queued_path.empty())
						{
							auto& pos = level.m_registry.get<c_position>(level.m_player);

							auto target = level.m_queued_path.back();
							level.m_queued_path.pop_back();

							if (!level.move_actor(level.m_player, pos, target))
							{
								bump::log_info("There is something in the way.");
								level.m_queued_path.clear();
							}
						}

						level.m_registry.get<c_actor>(level.m_player).take_turn_energy();
					}

					// monster turn
					{
						auto view = level.m_registry.view<c_actor, c_monster_tag, c_position>();

						for (auto m : view)
						{
							auto [p, a] = view.get<c_position, c_actor>(m);

							if (!a.has_turn_energy())
								continue;
							
							bump::log_info("monster turn!");
							
							monster_move(level, m, p, rng);

							a.take_turn_energy();
						}
					}
				}
			}
			
			// drawing
			{
				screen.buffer().fill(screen_cell_blank);

				draw_player_char_info(screen.buffer(), level.m_registry.get<c_player_char_info>(level.m_player), ui_main.m_py_name_sb, ui_main.m_py_title_sb);
				draw_player_xp(screen.buffer(), level.m_registry.get<c_xp>(level.m_player), ui_main.m_py_level_sb, ui_main.m_py_exp_sb);
				draw_player_stats(screen.buffer(), level.m_registry.get<c_stats>(level.m_player), ui_main.m_py_stats_sb);
				draw_player_hp(screen.buffer(), level.m_registry.get<c_hp>(level.m_player), ui_main.m_py_hp_sb);
				draw_player_sp(screen.buffer(), level.m_registry.get<c_sp>(level.m_player), ui_main.m_py_sp_sb);

				draw_level(screen.buffer(), level, ui_main.m_map_sb);
			}

			// render
			{
				auto& window = app.m_window;
				auto& renderer = app.m_renderer;

				renderer.clear_color_buffers({ 0.f, 0.f, 0.f, 1.f });
				renderer.clear_depth_buffers();
				renderer.set_viewport({ 0, 0 }, glm::uvec2(window.get_size()));

				screen.render(renderer);

				window.swap_buffers();
			}

			timer.tick();
		}
	
		bump::log_info("main loop - exit");
	}

} // rog
