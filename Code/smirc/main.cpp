
#include "smirc.hpp"

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_log.hpp>

int main(int , char*[] )
{

	{
		auto const metadata = bump::asset_metadata
		{
			// fonts
			{
				{ "menu", "RobotoMono-SemiBold.ttf", 30 },
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
				{ "ui_quad", { "ui_quad.vert", "ui_quad.frag" } },
				{ "ui_textured_quad", { "ui_textured_quad.vert", "ui_textured_quad.frag" } },
				{ "ui_label", { "ui_label.vert", "ui_label.frag" } },
			},
			// models
			{
				// { "skybox", "skybox.mbp_model" },
			},
			// 2d textures
			{
				{ "plus", "plus.png", { GL_RGBA8, GL_RGBA } },
			},
			// 2d array textures
			{
				// { "ascii_tiles", "ascii_tiles.png", 256, { GL_R8, GL_RED } },
			},
			// cubemaps
			{
				// { "skybox", { "skybox_x_pos.png", "skybox_x_neg.png", "skybox_y_pos.png", "skybox_y_neg.png", "skybox_z_pos.png", "skybox_z_neg.png" }, { GL_SRGB, GL_RGB, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR } },
			},
		};

		auto app = bump::app(metadata, { 1024, 768 }, "smirc", bump::sdl::window::display_mode::WINDOWED);
		bump::run_state({ [] (bump::app& app) { return smirc::main_state(app); } }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}