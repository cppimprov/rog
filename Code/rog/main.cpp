
#include "rog_colors.hpp"
#include "rog_direction.hpp"
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

	void move_player(player& player, bump::grid2<feature> const& level, direction dir)
	{
		auto const vec = get_direction_vector(dir);
		auto const level_size = level.extents();

		bump::die_if(level_size.x == 0 || level_size.y == 0);
		bump::die_if(player.m_position.x >= level_size.x || player.m_position.y >= level_size.y);

		if (player.m_position.x == 0 && vec.x < 0) return;
		if (player.m_position.x == level_size.x - 1 && vec.x > 0) return;
		if (player.m_position.y == 0 && vec.y < 0) return;
		if (player.m_position.y == level_size.y - 1 && vec.y > 0) return;

		auto const target = player.m_position + glm::size2(vec);

		if (level.at(target).m_flags & feature::flags::NO_WALK)
			return;
		
		player.m_position = target;
	}

	bool use_stairs(player& player, bump::grid2<feature> const& level, stairs_direction dir)
	{
		auto const level_size = level.extents();

		bump::die_if(level_size.x == 0 || level_size.y == 0);
		bump::die_if(player.m_position.x >= level_size.x || player.m_position.y >= level_size.y);

		if (dir == stairs_direction::UP)
		{
			if (!(level.at(player.m_position).m_flags & feature::flags::STAIRS_UP))
			{
				bump::log_info("There are no upward stairs here.");
				return false;
			}

			return true;
		}
		else
		{
			if (!(level.at(player.m_position).m_flags & feature::flags::STAIRS_DOWN))
			{
				bump::log_info("There are no downward stairs here.");
				return false;
			}

			return true;
		}
	}

	bump::gamestate play_level(
		bump::app& app, 
		std::queue<bump::input::input_event>& input_events, 
		bump::grid2<screen::cell>& screen_buffer, 
		bool const& request_quit,
		rog::player& player,
		std::int32_t level_depth)
	{
		using namespace bump;

		auto level = level_gen::generate_level(level_depth);
		
		while (true)
		{
			// todo: notify and wait

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

					     if (key.m_key == input::keyboard_key::NUM7 && key.m_value) move_player(player, level.m_grid, direction::LEFT_UP);
					else if (key.m_key == input::keyboard_key::NUM8 && key.m_value) move_player(player, level.m_grid, direction::UP);
					else if (key.m_key == input::keyboard_key::NUM9 && key.m_value) move_player(player, level.m_grid, direction::RIGHT_UP);
					else if (key.m_key == input::keyboard_key::NUM4 && key.m_value) move_player(player, level.m_grid, direction::LEFT);
					else if (key.m_key == input::keyboard_key::NUM6 && key.m_value) move_player(player, level.m_grid, direction::RIGHT);
					else if (key.m_key == input::keyboard_key::NUM1 && key.m_value) move_player(player, level.m_grid, direction::LEFT_DOWN);
					else if (key.m_key == input::keyboard_key::NUM2 && key.m_value) move_player(player, level.m_grid, direction::DOWN);
					else if (key.m_key == input::keyboard_key::NUM3 && key.m_value) move_player(player, level.m_grid, direction::RIGHT_DOWN);

					else if (key.m_key == input::keyboard_key::DOT && key.m_value && 
							app.m_input_handler.is_keyboard_key_pressed(bump::input::keyboard_key::LEFTSHIFT) && 
							use_stairs(player, level.m_grid, stairs_direction::DOWN))
						change_level = stairs_direction::DOWN;
					else if (key.m_key == input::keyboard_key::COMMA && key.m_value && 
							app.m_input_handler.is_keyboard_key_pressed(bump::input::keyboard_key::LEFTSHIFT) && 
							use_stairs(player, level.m_grid, stairs_direction::UP))
						change_level = stairs_direction::UP;
					
					else if (key.m_key == input::keyboard_key::ESCAPE && key.m_value)
						return { };

					continue;
				}
			}
			
			if (change_level)
			{
				auto const next_depth = level_depth + (change_level == stairs_direction::UP ? 1 : -1);
				return { [&, next_depth] (bump::app& app) { return play_level(app, input_events, screen_buffer, request_quit, player, next_depth); } };
			}

			// drawing!
			{
				screen::fill(screen_buffer, { ' ', colors::black, colors::black });
				screen::draw(screen_buffer, level.m_grid, player);
			}
		}

		return { };
	}

	void play_game(
		bump::app& app, 
		std::queue<bump::input::input_event>& input_events, 
		bump::grid2<screen::cell>& screen_buffer, 
		bool const& request_quit,
		bool& game_thread_done)
	{
		using namespace bump;

		auto player = rog::player(glm::size2(0), { '@', colors::yellow, colors::black });

		auto state_wrapper = bump::gamestate{ [&] (bump::app& app) { return play_level(app, input_events, screen_buffer, request_quit, player, 1); } };
		bump::run_state(state_wrapper, app);

		game_thread_done = true;
		// todo: notify and wait
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
			auto game_thread = std::thread(play_game, std::ref(app), std::ref(input_events), std::ref(screen_buffer), std::cref(request_quit), std::ref(game_thread_done));

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

				// todo: notify and wait
				
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
		
			// todo: notify and wait

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
