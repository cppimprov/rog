
#include <bump_app.hpp>
#include <bump_assets.hpp>
#include <bump_gamestate.hpp>
#include <bump_log.hpp>

#include <SDL.h>
#include <SDL_main.h>

#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>

#include <bump_camera.hpp>
#include <bump_gl.hpp>
#include <bump_grid.hpp>
#include <bump_input.hpp>
#include <bump_timer.hpp>
#include <bump_transform.hpp>
#include <bump_range.hpp>
#include <bump_render_text.hpp>

#include "rog_colors.hpp"
#include "rog_screen_buffer.hpp"
#include "rog_tile_renderer.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/std_based_type.hpp>

namespace rog
{

	bump::gamestate do_start(bump::app& app)
	{
		using namespace bump;

		log_info("start state");

		auto const tile_size = glm::ivec2{ 24, 36 };
		auto tile_renderer = rog::tile_renderer(app, glm::vec2(tile_size));

		auto screen = screen_buffer({ 80, 24 }, { '#', colors::light_red, colors::dark_red });
		screen.fill_rect({ 1, 1 }, screen.get_size() - glm::size2(2), { 'y', colors::light_green, colors::black });

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
				// callbacks.m_resize = [&] (glm::ivec2 window_size)
				// {
				// 	auto const num_tiles = window_size + (tile_size - 1) / tile_size;
				// 	screen.resize(glm::size2(num_tiles), { ' ', glm::vec3(1.0), glm::vec3(1.0, 0.0, 1.0) });
				// };

				app.m_input_handler.poll_input(callbacks);

				if (quit)
					return { };
			}

			// update
			{
				// auto const dt = paused ? frame_timer::clock_t::duration{ 0 } : timer.get_last_frame_time();

				// ...
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

				tile_renderer.render(renderer, window_size_f, screen);

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

// todo: 

	// fix font / tile spacing
		// warn if line height is smaller than box size.
		// render glyphs normally (no size checking?)
		// check all glyphs are smaller than tile size before blitting.
		// blit to final image (size check bounds of image, but not tiles?)

	// resize issue:
		// does the initial resize call change the size of the window?
		// ignore resizes that don't change the window size.

	// check frame-rate!

	// world representation
		// 
