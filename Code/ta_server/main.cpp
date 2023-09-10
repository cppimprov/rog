
#include <ta.hpp>
#include <ta_collision.hpp>
#include <ta_sprite.hpp>

#include <bump_app.hpp>
#include <bump_camera.hpp>
#include <bump_gamestate.hpp>
#include <bump_grid.hpp>
#include <bump_input.hpp>
#include <bump_log.hpp>
#include <bump_timer.hpp>
#include <bump_transform.hpp>

#include <iostream>
#include <map>
#include <queue>
#include <ranges>
#include <string>
#include <system_error>
#include <vector>

namespace ta
{
	
	enum tile_flags : std::uint8_t
	{
		none = 0,
		passable = 1 << 0,
		shootable = 1 << 1,
	};

	bump::gamestate main_loop(bump::app& app)
	{
		bump::log_info("main_loop()");

		auto const max_hp = ta::hp_t{ 100 };
		auto const bullet_damage = ta::hp_t{ 10 };
		auto const player_speed = 200.f;
		auto const bullet_speed = 500.f;
		auto const player_radius = 32.f;
		auto const bullet_radius = 4.f;
		auto const powerup_radius = 16.f;
		auto const tile_radius = 32.f;
		auto const powerup_duration = 5.f;
		auto const bullet_lifetime = 3.f;

		auto const tile_textures = std::vector<bump::gl::texture_2d const*>
		{
			&app.m_assets.m_textures_2d["grass"],
			&app.m_assets.m_textures_2d["road_ew"],
			&app.m_assets.m_textures_2d["road_ns"],
			&app.m_assets.m_textures_2d["road_cross"],
			&app.m_assets.m_textures_2d["building"],
			&app.m_assets.m_textures_2d["rubble"],
			&app.m_assets.m_textures_2d["water"],
		};

		auto const tile_symbols = std::map<char, std::size_t>
		{
			{ ' ', 0 },
			{ '-', 1 },
			{ '|', 2 },
			{ '+', 3 },
			{ '#', 4 },
			{ 'x', 5 },
			{ '~', 6 },
		};

		auto const tile_flags = std::vector<int>
		{
			tile_flags::passable | tile_flags::shootable,
			tile_flags::passable | tile_flags::shootable,
			tile_flags::passable | tile_flags::shootable,
			tile_flags::passable | tile_flags::shootable,
			tile_flags::none,
			tile_flags::passable | tile_flags::shootable,
			tile_flags::shootable,
		};

		using namespace std::string_view_literals;

		auto const symbol_map = 
		{
			"   xxxxx    ###~"sv,
			"------+-----###~"sv,
			"##### |  xx ###~"sv,
			"~~~  x|  xx~~ ~~"sv,
			"~~~ ##+----+----"sv,
			" ##  x|~##~|##  "sv,
			" ##  x|~##~+--+#"sv,
			"  +---+----+#x|#"sv,
			"  |#xx|x## +--+#"sv,
			"# |###| ##x|x###"sv,
			"--+---+----+----"sv,
			" #### ~~~ ##xx  "sv,
		};

		auto map_grid = bump::grid<std::size_t, 2>({ 16, 12 });

		auto map_y = std::size_t{ 0 };

		for (auto const row : std::ranges::reverse_view(symbol_map))
		{
			auto map_x = std::size_t{ 0 };

			for (auto const symbol : row)
			{
				map_grid.at({ map_x, map_y }) = tile_symbols.at(symbol);
				++map_x;
			}

			++map_y;
		}

		auto world = ta::world{ glm::vec2{ 0.f, 0.f }, glm::vec2(map_grid.extents()) * tile_radius * 2.f };

		world.m_players.push_back(ta::player{ 0, max_hp, glm::vec2{ 050.f, player_radius }, ta::direction::right, false, glm::vec3(1.f, 0.8f, 0.3f) });
		world.m_players.push_back(ta::player{ 1, max_hp, glm::vec2{ 150.f, player_radius }, ta::direction::right, false, glm::vec3(1.f, 0.f, 0.f) });
		world.m_players.push_back(ta::player{ 2, max_hp, glm::vec2{ 250.f, player_radius }, ta::direction::right, false, glm::vec3(0.f, 0.9f, 0.f) });
		world.m_players.push_back(ta::player{ 3, max_hp, glm::vec2{ 350.f, player_radius }, ta::direction::right, false, glm::vec3(0.2f, 0.2f, 1.f)});

		world.m_powerups.push_back(ta::powerup{ ta::powerup_type::bullet_speed, glm::vec2{ 250.f, 250.f }, glm::vec3(1.f, 0.f, 0.f), 5.f });

		auto tile_renderable = ta::tile_renderable(app.m_assets.m_shaders["sprite"]);
		auto tank_renderable = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["tank"], app.m_assets.m_textures_2d["tank_accent"]);
		auto tank_renderable_diagonal = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["tank_diagonal"], app.m_assets.m_textures_2d["tank_accent_diagonal"]);
		auto bullet_renderable = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["bullet"], app.m_assets.m_textures_2d["bullet_accent"]);
		auto powerup_renderble = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["powerup"], app.m_assets.m_textures_2d["powerup_accent"]);

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

						auto const dir = ta::get_input_dir(input_up, input_down, input_left, input_right);

						player.m_moving = dir.has_value();
						player.m_direction = dir.value_or(player.m_direction);
						
						if (input_fire)
						{
							if (reload_timer.get_elapsed_time() >= reload_time)
							{
								auto pos = player.m_position + dir_to_vec(player.m_direction) * player_radius;
								world.m_bullets.push_back({ player.m_id, pos, player.m_direction, player.m_color, bullet_speed, bullet_damage, bullet_lifetime });
								reload_timer = bump::timer();
							}
						}
					}
				}

				// update player positions
				for (auto& player : world.m_players)
				{
					if (!player.m_moving)
						continue;

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

					if (result.m_collided)
						bullet.m_lifetime = 0.f;
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

						if (dist < (player_radius + powerup_radius))
						{
							player.m_powerup_timers[powerup.m_type] = powerup_duration;
							powerup.m_lifetime = 0.f;
						}
					}
				}

				// update powerup lifetimes
				for (auto& powerup : world.m_powerups)
					powerup.m_lifetime -= delta_time;

				// check for expired powerups
				std::erase_if(world.m_powerups,
					[] (auto const& p) { return p.m_lifetime <= 0.f; });

				// check for player damage
				for (auto& player : world.m_players)
				{
					for (auto& bullet : world.m_bullets)
					{
						if (bullet.m_owner_id == player.m_id)
							continue;

						auto const dist = glm::distance(player.m_position, bullet.m_position);

						if (dist < player_radius + bullet_radius)
						{
							player.m_hp -= bullet.m_damage;
							bullet.m_lifetime = 0.f;
						}
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

				app.m_renderer.set_blending(bump::gl::renderer::blending::BLEND);

				for (auto y : bump::range(0, map_grid.extents()[1]))
				{
					for (auto x : bump::range(0, map_grid.extents()[0]))
					{
						auto const tile_index = map_grid.at({ x, y });
						auto const tile_texture = tile_textures[tile_index];

						auto const position = glm::vec2(tile_radius) + glm::vec2(x * tile_radius * 2.f, y * tile_radius * 2.f);

						auto model_matrix = glm::mat4(1.f);
						bump::set_position(model_matrix, glm::vec3(position, 0.f));

						tile_renderable.render(app.m_renderer, *tile_texture, matrices, model_matrix, glm::vec2(tile_radius * 2.f));
					}
				}

				for (auto const& p : world.m_players)
				{
					auto const rotation_angle = dir_to_angle(p.m_direction);

					auto model_matrix = glm::mat4(1.f);
					bump::set_position(model_matrix, glm::vec3(p.m_position, 0.f));
					bump::set_rotation(model_matrix, glm::angleAxis(glm::radians(rotation_angle), glm::vec3(0.f, 0.f, 1.f)));

					auto& renderable = is_diagonal(p.m_direction) ? tank_renderable_diagonal : tank_renderable;
					renderable.render(app.m_renderer, matrices, model_matrix, glm::vec2(player_radius * 2.f), p.m_color);
				}

				for (auto const& b : world.m_bullets)
				{
					auto model_matrix = glm::mat4(1.f);
					bump::set_position(model_matrix, glm::vec3(b.m_position, 0.f));
					bullet_renderable.render(app.m_renderer, matrices, model_matrix, glm::vec2(bullet_radius * 2.f), b.m_color);
				}
					
				for (auto const& p : world.m_powerups)
				{
					auto model_matrix = glm::mat4(1.f);
					bump::set_position(model_matrix, glm::vec3(p.m_position, 0.f));
					powerup_renderble.render(app.m_renderer, matrices, model_matrix, glm::vec2(powerup_radius * 2.f), p.m_color);
				}
				
				app.m_renderer.set_blending(bump::gl::renderer::blending::NONE);

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
		bump::run_state({ [] (bump::app& app) { return ta::main_loop(app); } }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}

// todo:

	// world:
		// building collision (players / bullets)
		// water (impassable, but can shoot over)

	// implement powerup functionality

	// ... finally ...
	// start working on server code!
