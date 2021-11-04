
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

#include <bump_gl.hpp>
#include <bump_grid.hpp>
#include <bump_input.hpp>
#include <bump_timer.hpp>
#include <bump_range.hpp>
#include <bump_render_text.hpp>

#include "rog_screen_buffer.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/std_based_type.hpp>

namespace rog
{

	bump::gamestate do_start(bump::app& app)
	{
		using namespace bump;

		log_info("start state");

		auto screen = screen_buffer({ 80, 24 }, '#');
		screen.fill_rect({ 1, 1 }, screen.get_size() - glm::size2(2), '.');

		auto const ascii_tiles = render_ascii_tiles(app.m_ft_context, app.m_assets.m_fonts.at("tiles"), { 64, 64 });

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
				// ...

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
				{ "tiles", "RobotoMono-SemiBold.ttf", 30 },
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
				// { "text_quad", { "text_quad.vert", "text_quad.frag" } },
			},
			// models
			{
				// { "skybox", "skybox.mbp_model" },
			},
			// cubemaps
			{
				// { "skybox", { "skybox_x_pos.png", "skybox_x_neg.png", "skybox_y_pos.png", "skybox_y_neg.png", "skybox_z_pos.png", "skybox_z_neg.png" } },
			},
		};

		auto app = bump::app(metadata, { 1024, 768 }, "rog", bump::sdl::window::display_mode::WINDOWED);
		bump::run_state({ rog::do_start }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}

// todo: 

	// basic font rendering:

		// rendering
			// tile_renderable class (instanced quad rendering)
			// shader

	// build script:
		// rc stage to add icon (yellow @ sign)
