
#include <ta.hpp>
#include <ta_collision.hpp>
#include <ta_sprite.hpp>

#include <bump_app.hpp>
#include <bump_camera.hpp>
#include <bump_gamestate.hpp>
#include <bump_input.hpp>
#include <bump_log.hpp>
#include <bump_timer.hpp>
#include <bump_transform.hpp>

#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <system_error>

#include <iostream>

namespace ta
{

	bump::gamestate main_loop(bump::app& app)
	{
		bump::log_info("main_loop()");

		auto const max_hp = ta::hp_t{ 100 };
		auto const bullet_damage = ta::hp_t{ 10 };
		auto const player_speed = 200.f;
		auto const bullet_speed = 500.f;
		auto const player_radius = 30.f;
		auto const bullet_radius = 5.f;
		auto const powerup_radius = 25.f;
		auto const powerup_duration = 10.f;
		auto const bullet_lifetime = 3.f;

		auto world = ta::world{ glm::vec2{ 0.f, 0.f }, glm::vec2{ 1000.f, 1000.f } };
		world.m_players.push_back(ta::player{ 0, max_hp, glm::vec2{ 000.f, player_radius }, ta::direction::none, glm::vec3(1.f, 0.8f, 0.3f) });
		world.m_players.push_back(ta::player{ 1, max_hp, glm::vec2{ 100.f, player_radius }, ta::direction::none, glm::vec3(1.f, 0.f, 0.f) });
		world.m_players.push_back(ta::player{ 2, max_hp, glm::vec2{ 200.f, player_radius }, ta::direction::none, glm::vec3(0.f, 0.9f, 0.f) });
		world.m_players.push_back(ta::player{ 3, max_hp, glm::vec2{ 300.f, player_radius }, ta::direction::none, glm::vec3(0.2f, 0.2f, 1.f)});

		auto tank_renderable = ta::tank_renderable(app.m_assets.m_shaders["tank"]);
		auto bullet_renderable = ta::bullet_renderable(app.m_assets.m_shaders["bullet"]);

		auto app_events = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();

		auto timer = bump::frame_timer();
		auto reload_timer = bump::timer();
		auto const reload_time = std::chrono::duration<float>(0.2f);

		auto input_up = false;
		auto input_down = false;
		auto input_left = false;
		auto input_right = false;
		auto input_fire = false;

		while (true)
		{
			// get input
			{
				app.m_input_handler.poll(app_events, input_events);

				while (!app_events.empty())
				{
					auto event = std::move(app_events.front());
					app_events.pop();

					namespace ae = bump::input::app_events;

					if (std::holds_alternative<ae::quit>(event))
						return { };	// quit
				}

				while (!input_events.empty())
				{
					auto event = std::move(input_events.front());
					input_events.pop();

					namespace ie = bump::input::input_events;

					if (std::holds_alternative<ie::keyboard_key>(event))
					{
						auto const& k = std::get<ie::keyboard_key>(event);

						using kt = bump::input::keyboard_key;

						if (k.m_key == kt::ESCAPE && k.m_value)
							return { }; // quit
						
						if (k.m_key == kt::W) input_up = k.m_value;
						if (k.m_key == kt::S) input_down = k.m_value;
						if (k.m_key == kt::A) input_left = k.m_value;
						if (k.m_key == kt::D) input_right = k.m_value;
						if (k.m_key == kt::SPACE) input_fire = k.m_value;
					}
				}
			}
			
			// update
			{
				auto const delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(timer.get_last_frame_time()).count();

				// update player input
				{
					// player input

					auto p = std::find_if(world.m_players.begin(), world.m_players.end(),
						[] (auto const& p) { return p.m_id == 0; });

					if (p != world.m_players.end())
					{
						auto& player = *p;

						player.m_direction = ta::get_input_dir(input_up, input_down, input_left, input_right);
						
						// todo: proper bullet spawn position!
						// todo: proper bullet velocity!
						if (input_fire)
						{
							if (reload_timer.get_elapsed_time() >= reload_time)
							{
								world.m_bullets.push_back({ player.m_id, player.m_position, player.m_direction, player.m_color, bullet_speed, bullet_damage, bullet_lifetime });
								reload_timer = bump::timer();
							}
						}
					}
				}

				// update player positions
				for (auto& player : world.m_players)
				{
					auto result = ta::check_collision(world, { player.m_position, player.m_direction, player_radius, player_speed }, false, delta_time);
					player.m_position = result.m_position;
					player.m_direction = result.m_direction;
				}

				// update bullet positions
				for (auto& bullet : world.m_bullets)
				{
					auto result = ta::check_collision(world, { bullet.m_position, bullet.m_direction, bullet_radius, bullet_speed }, false, delta_time);
					bullet.m_position = result.m_position;
					bullet.m_direction = result.m_direction;
				}

				// update bullet lifetimes
				for (auto& bullet : world.m_bullets)
					bullet.m_lifetime -= delta_time;

				// remove expired bullets
				std::erase_if(world.m_bullets,
					[] (auto const& b) { return b.m_lifetime <= 0.f; });

				// update powerup timers
				for (auto& player : world.m_players)
				{
					for (auto& powerup_timer : player.m_powerup_timers)
						powerup_timer.second -= delta_time;
						
					// remove expired powerups
					std::erase_if(player.m_powerup_timers,
						[] (auto const& p) { return p.second <= 0.f; });
				}

				// check for powerup pickups
				for (auto& player : world.m_players)
				{
					for (auto& powerup : world.m_powerups)
					{
						auto const dist = glm::distance(player.m_position, powerup.m_position);

						if (dist < player_radius + powerup_radius)
							player.m_powerup_timers[powerup.m_type] = powerup_duration;
					}
				}

				// check for player damage
				for (auto& player : world.m_players)
				{
					for (auto& bullet : world.m_bullets)
					{
						if (bullet.m_owner_id == player.m_id)
							continue;

						auto const dist = glm::distance(player.m_position, bullet.m_position);

						if (dist < player_radius + bullet_radius)
							player.m_hp -= bullet.m_damage;
					}
				}

				// check for player deaths
				std::erase_if(world.m_players,
					[] (auto const& p) { return p.m_hp <= 0; });
			}

			// render
			{
				app.m_renderer.clear_color_buffers({ 0.39215f, 0.58431f, 0.92941f, 1.f });
				app.m_renderer.clear_depth_buffers();

				app.m_renderer.set_viewport({ 0, 0 }, glm::uvec2(app.m_window.get_size()));

				auto camera = bump::orthographic_camera();
				camera.m_projection.m_size = glm::vec2(app.m_window.get_size());
				camera.m_viewport.m_size = glm::vec2(app.m_window.get_size());
				
				auto const matrices = bump::camera_matrices(camera);

				// render world
				// render powerups

				for (auto const& p : world.m_players)
					tank_renderable.render(app.m_renderer, matrices, p.m_position - glm::vec2(player_radius), glm::vec2(player_radius * 2.f), p.m_color);

				for (auto const& b : world.m_bullets)
					bullet_renderable.render(app.m_renderer, matrices, b.m_position - glm::vec2(bullet_radius), glm::vec2(bullet_radius * 2.f), b.m_color);

				app.m_window.swap_buffers();
			}

			timer.tick();
		}
	}

} // ta

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
				{ "tank", { "tank.vert", "tank.frag" } },
				{ "bullet", { "bullet.vert", "bullet.frag" } },
			},
			// models
			{
				// { "skybox", "skybox.mbp_model" },
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
		bump::run_state({ [] (bump::app& app) { return ta::main_loop(app); } }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}

// todo:

	// fix player directions -> shouldn't have a "none" direction (need a separate movement flag?)

	// render players:
		// add texture to tank renderable
		// add color to players / tank_renderable

	// render bullets:
		// get color from player

	// fix issues with bullets (see todos above)
	
	// instancing for renderables?

	// check that test project is set up for unit testing?
