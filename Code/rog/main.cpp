
#include <bump_app.hpp>
#include <bump_assets.hpp>
#include <bump_gamestate.hpp>
#include <bump_log.hpp>

#include <SDL.h>
#include <SDL_main.h>

#include <cstdlib>
#include <format>
#include <iostream>

#include <bump_input.hpp>
#include <bump_timer.hpp>

namespace rog
{

	bump::gamestate do_start(bump::app& app)
	{
		using namespace bump;

		log_info("start state");

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
				// { "press_start", "Bungee-Regular.ttf", 64 },
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

// todo: commit! (end of stream :( )
