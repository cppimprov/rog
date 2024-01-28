
#include <bump_app.hpp>
#include <bump_enet.hpp>
#include <bump_gamestate.hpp>
#include <bump_log.hpp>

#include "ta_state.hpp"

int main(int , char* [])
{
	{
		auto metadata = bump::asset_metadata
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
				{ "sprite", { "sprite.vert", "sprite.frag" } },
				{ "sprite_accent", { "sprite_accent.vert", "sprite_accent.frag" } },
			},
			// models
			{
				// { "skybox", "skybox.mbp_model" },
			},
			// 2d textures
			{
				{ "grass", "grass.png", { GL_RGBA8, GL_RGBA } },
				{ "road_ew", "road_ew.png", { GL_RGBA8, GL_RGBA } },
				{ "road_ns", "road_ns.png", { GL_RGBA8, GL_RGBA } },
				{ "road_cross", "road_cross.png", { GL_RGBA8, GL_RGBA } },
				{ "building", "building.png", { GL_RGBA8, GL_RGBA } },
				{ "rubble", "rubble.png", { GL_RGBA8, GL_RGBA } },
				{ "water", "water.png", { GL_RGBA8, GL_RGBA } },

				{ "tank", "tank_color.png", { GL_RGBA8, GL_RGBA } },
				{ "tank_accent", "tank_mask.png", { GL_RGBA8, GL_RGBA } },
				{ "tank_diagonal", "tank_color_diagonal.png", { GL_RGBA8, GL_RGBA } },
				{ "tank_accent_diagonal", "tank_mask_diagonal.png", { GL_RGBA8, GL_RGBA } },

				{ "bullet", "bullet_color.png", { GL_RGBA8, GL_RGBA } },
				{ "bullet_accent", "bullet_mask.png", { GL_RGBA8, GL_RGBA } },
				
				{ "powerup", "powerup_color.png", { GL_RGBA8, GL_RGBA } },
				{ "powerup_accent", "powerup_mask.png", { GL_RGBA8, GL_RGBA } },
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

		auto app = bump::app(metadata, { 1024, 768 }, "ta_server", bump::sdl::window::display_mode::WINDOWED);
		app.m_gl_context.set_swap_interval(bump::sdl::gl_context::swap_interval_mode::ADAPTIVE_VSYNC);

		bump::run_state({ [] (bump::app& app) { return ta::loading(app); } }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}

// todo: use the fixed timestep in the waiting / connecting states...

// todo: networking
	// send input to server
	// get player positions from server
	// interpolate / extrapolate player positions

// todo: split gamestates into smaller functions as necessary

// todo (sometime): not all apps need network code... (or other things)
	// each program should have its own "app" class that includes the things it needs
	// then we can remove the bump::app class
