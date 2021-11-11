
#include "rog_color.hpp"
#include "rog_feature.hpp"
#include "rog_player.hpp"
#include "rog_screen_drawing.hpp"
#include "rog_tile_renderer.hpp"

#include <bump_app.hpp>
#include <bump_assets.hpp>
#include <bump_camera.hpp>
#include <bump_gamestate.hpp>
#include <bump_gl.hpp>
#include <bump_grid.hpp>
#include <bump_input.hpp>
#include <bump_log.hpp>
#include <bump_timer.hpp>
#include <bump_transform.hpp>
#include <bump_range.hpp>
#include <bump_render_text.hpp>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/std_based_type.hpp>
#include <glm/gtx/string_cast.hpp>

#include <SDL.h>
#include <SDL_main.h>

#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>

namespace rog
{

	enum class direction
	{
		LEFT_UP,
		UP,
		RIGHT_UP,
		LEFT,
		NONE,
		RIGHT,
		LEFT_DOWN,
		DOWN,
		RIGHT_DOWN,
	};

	glm::ivec2 get_direction_vector(direction dir)
	{
		switch (dir)
		{
		case direction::LEFT_UP:    return { -1, -1 };
		case direction::UP:         return {  0, -1 };
		case direction::RIGHT_UP:   return {  1, -1 };
		case direction::LEFT:       return { -1,  0 };
		case direction::NONE:       return {  0,  0 };
		case direction::RIGHT:      return {  1,  0 };
		case direction::LEFT_DOWN:  return { -1,  1 };
		case direction::DOWN:       return {  0,  1 };
		case direction::RIGHT_DOWN: return {  1,  1 };
		}

		bump::die();
	}

	void move_player(player& player, bump::grid2<feature> const& level, direction dir)
	{
		auto const vec = get_direction_vector(dir);
		auto const level_size = level.extents();

		if (player.m_position.x == 0 && vec.x < 0) return;
		if (player.m_position.x == level_size.x - 1 && vec.x > 0) return;
		if (player.m_position.y == 0 && vec.y < 0) return;
		if (player.m_position.y == level_size.y - 1 && vec.y > 0) return;
		
		player.m_position += vec;
	}

	bump::gamestate do_start(bump::app& app)
	{
		using namespace bump;

		log_info("start state");

		auto const tile_size = glm::ivec2{ 24, 36 };
		auto tile_renderer = rog::tile_renderer(app, glm::vec2(tile_size));

		auto screen_buffer = grid2<screen::cell>();
		screen::resize(screen_buffer, app.m_window.get_size(), tile_size, { '#', color::light_red, color::dark_red });

		auto level_grid = grid2<feature>({ 128, 128 }, { { '#', color::white, color::dark_grey } });

		for (auto y : bump::range(1, 127))
			for (auto x : bump::range(1, 127))
				level_grid.at({ x, y }) = { { '.', color::white, color::black } };
		
		auto player = rog::player(glm::size2(0), { '@', color::yellow, color::black });

		auto paused = false;
		auto timer = frame_timer();

		while (true)
		{
			// input
			{
				auto quit = false;
				auto callbacks = input::input_callbacks();
				callbacks.m_quit = [&] () { quit = true; };
				callbacks.m_pause = [&] (bool pause) { paused = pause; if (!paused) timer = frame_timer(); };
				callbacks.m_resize = [&] (glm::ivec2 window_size) { screen::resize(screen_buffer, window_size, tile_size, { ' ', glm::vec3(1.0), glm::vec3(1.0, 0.0, 1.0) }); };
				callbacks.m_input = [&] (input::control_id id, input::raw_input in)
				{
					if (id == input::control_id::KEYBOARDKEY_NUM7 && in.m_value) move_player(player, level_grid, direction::LEFT_UP);
					if (id == input::control_id::KEYBOARDKEY_NUM8 && in.m_value) move_player(player, level_grid, direction::UP);
					if (id == input::control_id::KEYBOARDKEY_NUM9 && in.m_value) move_player(player, level_grid, direction::RIGHT_UP);
					if (id == input::control_id::KEYBOARDKEY_NUM4 && in.m_value) move_player(player, level_grid, direction::LEFT);
					if (id == input::control_id::KEYBOARDKEY_NUM6 && in.m_value) move_player(player, level_grid, direction::RIGHT);
					if (id == input::control_id::KEYBOARDKEY_NUM1 && in.m_value) move_player(player, level_grid, direction::LEFT_DOWN);
					if (id == input::control_id::KEYBOARDKEY_NUM2 && in.m_value) move_player(player, level_grid, direction::DOWN);
					if (id == input::control_id::KEYBOARDKEY_NUM3 && in.m_value) move_player(player, level_grid, direction::RIGHT_DOWN);
				};

				app.m_input_handler.poll_input(callbacks);

				if (quit)
					return { };
			}

			// update
			{
				// auto const dt = paused ? frame_timer::clock_t::duration{ 0 } : timer.get_last_frame_time();

				// ...

				// drawing!
				screen::fill(screen_buffer, { ' ', color::black, color::black });
				screen::draw(screen_buffer, level_grid, player);
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
		bump::run_state({ rog::do_start }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}

// todo (now):

	// level:
		// generation!

// todo (sometime):
	
	// organising:
		// fill, fill_area, should work for any grid.




// notes:

	// For a terminal-style roguelike, we can't "center" a tile exactly in the middle of the 
	// screen, because we use the same grid for the ui (and we need the ui to have a consistent position, and use whole tiles)
	// if we separate the world / ui, then we don't have to do that.

	// Centering the player would be much better for mouse input (because we don't
	// have to move the mouse as much to "undo" the move we just made). Though perhaps
	// this would matter less in a "busy" environment.

	// ...