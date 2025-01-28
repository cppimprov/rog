
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
				{ "utf", "Arimo-Regular.ttf", 16 },
				{ "kr", "NotoSansKR-Regular.ttf", 16 },
				{ "title", "RobotoMono-SemiBold.ttf", 20 },
				{ "label", "RobotoMono-SemiBold.ttf", 8 },
				{ "field", "RobotoMono-SemiBold.ttf", 10 },
				{ "button", "RobotoMono-SemiBold.ttf", 10 },
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
				{ "ui_rect", { "ui_rect.vert", "ui_rect.frag" } },
				{ "ui_textured_rect", { "ui_textured_rect.vert", "ui_textured_rect.frag" } },
				{ "ui_text", { "ui_text.vert", "ui_text.frag" } },
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

// todo:

	// add profiles button in top-right, which opens the profiles dialog
	// add close / save button to profiles
	// change profile name to nick
	// make user / real default to nick, with checkbox to show the text boxes.

	// text alignment in label / label_button widgets

// features:

	// new connection dialog!
		// label title
		// label / text field pairs for input
		// tab to move forwards between input fields, shift-tab to move backwards
		// ...

	// ...

	// profile button + dialog!
		// profile button in top left
		// panel with title bar at the top
		// list of profiles on the left (with + button to add a new one)
		// clicking on a profile shows the profile
		// clicking on the new button shows a blank profile
		// fields for nickname, username, real name, email, button to delete?
		// ...