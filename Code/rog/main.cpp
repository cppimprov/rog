
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

	bump::gamestate play_level(bump::app& app, std::int32_t depth)
	{
		using namespace bump;

		log_info("start state");

		auto const tile_size = glm::ivec2{ 24, 36 };
		auto tile_renderer = rog::tile_renderer(app, glm::vec2(tile_size));

		auto screen_buffer = grid2<screen::cell>();
		screen::resize(screen_buffer, app.m_window.get_size(), tile_size, { '#', colors::light_red, colors::dark_red });

		auto level = level_gen::generate_level(depth);
		auto player = rog::player(glm::size2(0), { '@', colors::yellow, colors::black });

		auto paused = false;
		auto timer = frame_timer();

		auto shift_pressed = false;

		while (true)
		{
			// input
			{
				auto quit = false;
				auto change_level = std::optional<stairs_direction>();
				auto callbacks = input::input_callbacks();
				callbacks.m_quit = [&] () { quit = true; };
				callbacks.m_pause = [&] (bool pause) { paused = pause; if (!paused) timer = frame_timer(); };
				callbacks.m_resize = [&] (glm::ivec2 window_size) { screen::resize(screen_buffer, window_size, tile_size, { ' ', glm::vec3(1.0), glm::vec3(1.0, 0.0, 1.0) }); };
				callbacks.m_input = [&] (input::control_id id, input::raw_input in)
				{
					if (id == input::control_id::KEYBOARDKEY_NUM7 && in.m_value) move_player(player, level.m_grid, direction::LEFT_UP);
					if (id == input::control_id::KEYBOARDKEY_NUM8 && in.m_value) move_player(player, level.m_grid, direction::UP);
					if (id == input::control_id::KEYBOARDKEY_NUM9 && in.m_value) move_player(player, level.m_grid, direction::RIGHT_UP);
					if (id == input::control_id::KEYBOARDKEY_NUM4 && in.m_value) move_player(player, level.m_grid, direction::LEFT);
					if (id == input::control_id::KEYBOARDKEY_NUM6 && in.m_value) move_player(player, level.m_grid, direction::RIGHT);
					if (id == input::control_id::KEYBOARDKEY_NUM1 && in.m_value) move_player(player, level.m_grid, direction::LEFT_DOWN);
					if (id == input::control_id::KEYBOARDKEY_NUM2 && in.m_value) move_player(player, level.m_grid, direction::DOWN);
					if (id == input::control_id::KEYBOARDKEY_NUM3 && in.m_value) move_player(player, level.m_grid, direction::RIGHT_DOWN);

					if (id == input::control_id::KEYBOARDKEY_LEFTSHIFT || id == input::control_id::KEYBOARDKEY_RIGHTSHIFT) shift_pressed = in.m_value;
					if (id == input::control_id::KEYBOARDKEY_DOT && in.m_value && shift_pressed && use_stairs(player, level.m_grid, stairs_direction::DOWN))
						change_level = stairs_direction::DOWN;
					if (id == input::control_id::KEYBOARDKEY_COMMA && in.m_value && shift_pressed && use_stairs(player, level.m_grid, stairs_direction::UP))
						change_level = stairs_direction::UP;

					if (id == input::control_id::KEYBOARDKEY_ESCAPE && in.m_value)
						quit = true;
				};

				app.m_input_handler.poll_input(callbacks);

				if (quit)
					return { };

				if (change_level)
				{
					auto const next_depth = depth + (change_level == stairs_direction::UP ? 1 : -1);
					return { [next_depth] (bump::app& app) { return play_level(app, next_depth); } };
				}
			}

			// update
			{
				// auto const dt = paused ? frame_timer::clock_t::duration{ 0 } : timer.get_last_frame_time();

				// ...

				// drawing!
				screen::fill(screen_buffer, { ' ', colors::black, colors::black });
				screen::draw(screen_buffer, level.m_grid, player);
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
		
		return { };
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
		bump::run_state({ [] (bump::app& app) { return rog::play_level(app, 1); } }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}

// todo (now):

	// level generation:

		// create level_gen namespace / file.
		// create rect / area type.
		// add inside / outside subdivision type?
		// ...



// todo (sometime):
