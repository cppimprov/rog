
#include "ta_world.hpp"
#include "ta_globals.hpp"

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_log.hpp>
#include <bump_transform.hpp>

#include <random>

namespace ta
{

	bump::gamestate main_loop(bump::app& app)
	{
		bump::log_info("main_loop()");

		auto world = ta::world();
		auto world_physics = ta::world_physics{ b2World{ b2Vec2{ 0.f, 0.f } } };
		auto world_graphics = ta::world_graphics
		{
			.m_tile_textures = {
				&app.m_assets.m_textures_2d["grass"],
				&app.m_assets.m_textures_2d["road_ew"],
				&app.m_assets.m_textures_2d["road_ns"],
				&app.m_assets.m_textures_2d["road_cross"],
				&app.m_assets.m_textures_2d["building"],
				&app.m_assets.m_textures_2d["rubble"],
				&app.m_assets.m_textures_2d["water"],
			},

			.m_tile_renderable = ta::tile_renderable(app.m_assets.m_shaders["sprite"]),
			.m_tank_renderable = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["tank"], app.m_assets.m_textures_2d["tank_accent"]),
			.m_tank_renderable_diagonal = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["tank_diagonal"], app.m_assets.m_textures_2d["tank_accent_diagonal"]),
			.m_bullet_renderable = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["bullet"], app.m_assets.m_textures_2d["bullet_accent"]),
			.m_powerup_renderable = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["powerup"], app.m_assets.m_textures_2d["powerup_accent"]),
		};

		world.m_players.push_back(create_player(world.m_registry, world_physics.m_b2_world, globals::player_radius * glm::vec2{ 1.f, 8.f }, glm::vec3(1.f, 0.8f, 0.3f)));

		auto player_entity = entt::entity{ world.m_players.back() };
		world.m_registry.emplace<c_player_input>(player_entity);

		world.m_players.push_back(create_player(world.m_registry, world_physics.m_b2_world, globals::player_radius * glm::vec2{ 5.f, 3.f }, glm::vec3(1.f, 0.f, 0.f)));
		world.m_players.push_back(create_player(world.m_registry, world_physics.m_b2_world, globals::player_radius * glm::vec2{ 7.f, 3.f }, glm::vec3(0.f, 0.9f, 0.f)));
		world.m_players.push_back(create_player(world.m_registry, world_physics.m_b2_world, globals::player_radius * glm::vec2{ 9.f, 3.f }, glm::vec3(0.2f, 0.2f, 1.f)));

		load_test_map(world, world_physics);
		set_world_bounds(world_physics, glm::vec2(world.m_tiles.extents()) * globals::tile_radius * 2.f);

		auto app_events = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();

		auto rng = std::mt19937_64{ std::random_device{}() };
		auto tile_list = std::vector<entt::entity>();

		auto timer = bump::frame_timer();
		auto powerup_spawn_timer = bump::timer();

		while (true)
		{
			// input
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
						
						if (player_entity != entt::null)
						{
							auto& pi = world.m_registry.get<c_player_input>(player_entity);

							if (k.m_key == kt::W) pi.m_input_up = k.m_value;
							if (k.m_key == kt::S) pi.m_input_down = k.m_value;
							if (k.m_key == kt::A) pi.m_input_left = k.m_value;
							if (k.m_key == kt::D) pi.m_input_right = k.m_value;
							if (k.m_key == kt::SPACE) pi.m_input_fire = k.m_value;
						}
					}
				}
			}
			
			// update
			{
				auto const delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(timer.get_last_frame_time()).count();

				// update player input
				{
					if (player_entity != entt::null)
					{
						auto [pu, pp, pm, pi] = world.m_registry.get<c_player_powerups, c_player_physics, c_player_movement, c_player_input>(player_entity);

						auto const dir = ta::get_input_dir(pi.m_input_up, pi.m_input_down, pi.m_input_left, pi.m_input_right);

						pm.m_moving = dir.has_value();
						pm.m_direction = dir.value_or(pm.m_direction);

						if (pi.m_input_fire)
						{
							auto const reload_time = pu.m_timers.contains(powerup_type::player_reload_speed) ? globals::powerup_player_reload_speed : globals::reload_time;

							if (pi.m_reload_timer.get_elapsed_time() >= reload_time)
							{
								auto const speed_mul = pu.m_timers.contains(powerup_type::bullet_speed) ? globals::powerup_bullet_speed_multiplier : 1.f;
								auto const pos_px = (globals::b2_inv_scale_factor * to_glm_vec2(pp.m_b2_body->GetPosition())) + dir_to_vec(pm.m_direction) * globals::player_radius;
								auto const group_index = pp.m_b2_body->GetFixtureList()->GetFilterData().groupIndex;
								world.m_bullets.push_back(create_bullet(world.m_registry, world_physics.m_b2_world, player_entity, group_index, pos_px, dir_to_vec(pm.m_direction) * globals::bullet_speed * speed_mul));
								pi.m_reload_timer = bump::timer();
							}
						}
					}
				}

				// update player movement
				{
					auto const player_view = world.m_registry.view<c_player_physics, c_player_powerups, c_player_movement>();

					for (auto const p : player_view)
					{
						auto [pp, pu, pm] = player_view.get<c_player_physics, c_player_powerups, c_player_movement>(p);

						if (!pm.m_moving)
						{
							pp.m_b2_body->SetLinearVelocity(b2Vec2{ 0.f, 0.f });
						}
						else
						{
							auto const multiplier = pu.m_timers.contains(powerup_type::player_speed) ? globals::powerup_player_speed_multiplier : 1.f;
							auto const velocity_px = dir_to_vec(pm.m_direction) * globals::player_speed * multiplier;
							auto const b2_velocity = to_b2_vec2(globals::b2_scale_factor * velocity_px);
							pp.m_b2_body->SetLinearVelocity(b2_velocity);
						}
					}
				}

				// prepare collision callbacks
				struct contact_listener : public b2ContactListener
				{
					contact_listener(ta::world& world, ta::world_physics& world_physics):
						m_world(world),
						m_world_physics(world_physics)
					{ }

					void BeginContact(b2Contact* contact) override
					{
						// check types and handle each one appropriately
						auto const& fixture_a = *contact->GetFixtureA();
						auto const& body_a = *fixture_a.GetBody();
						auto const entity_a = static_cast<entt::entity>(body_a.GetUserData().pointer);
						auto const& fixture_b = *contact->GetFixtureB();
						auto const& body_b = *fixture_b.GetBody();
						auto const entity_b = static_cast<entt::entity>(body_b.GetUserData().pointer);

						auto const has_category = [] (b2Fixture const& fixture, collision_category category)
						{
							return (fixture.GetFilterData().categoryBits & category) != 0;
						};

						if (has_category(fixture_a, collision_category::player) && has_category(fixture_b, collision_category::bullet))
							return player_bullet(entity_a, entity_b);
						if (has_category(fixture_a, collision_category::bullet) && has_category(fixture_b, collision_category::player))
							return player_bullet(entity_b, entity_a);

						if (has_category(fixture_a, collision_category::player) && has_category(fixture_b, collision_category::powerup))
							return player_powerup(entity_a, entity_b);
						if (has_category(fixture_a, collision_category::powerup) && has_category(fixture_b, collision_category::player))
							return player_powerup(entity_b, entity_a);

						if (has_category(fixture_a, collision_category::bullet) && has_category(fixture_b, collision_category::tile_wall))
							return bullet_tile(entity_a, entity_b);
						if (has_category(fixture_a, collision_category::tile_wall) && has_category(fixture_b, collision_category::bullet))
							return bullet_tile(entity_b, entity_a);
						
						if (has_category(fixture_a, collision_category::bullet) && has_category(fixture_b, collision_category::world_bounds))
							return bullet_world_bounds(entity_a);
						if (has_category(fixture_a, collision_category::world_bounds) && has_category(fixture_b, collision_category::bullet))
							return bullet_world_bounds(entity_b);
					}

				private:

					void player_bullet(entt::entity player_entity, entt::entity bullet_entity)
					{
						auto& ph = m_world.m_registry.get<c_player_hp>(player_entity);
						auto [bo, bl] = m_world.m_registry.get<c_bullet_owner_id, c_bullet_lifetime>(bullet_entity);

						if (bo.m_owner_id == player_entity)
							return;

						auto const& opu = m_world.m_registry.get<c_player_powerups>(bo.m_owner_id);
						auto const damage_multiplier = opu.m_timers.contains(powerup_type::bullet_damage) ? globals::powerup_bullet_damage_multiplier : 1.f;

						ph.m_hp -= static_cast<std::uint32_t>(globals::bullet_damage * damage_multiplier);
						bl.m_lifetime = 0.f;
					}

					void player_powerup(entt::entity player_entity, entt::entity powerup_entity)
					{
						auto& pp = m_world.m_registry.get<c_player_powerups>(player_entity);
						auto [pt, pl] = m_world.m_registry.get<c_powerup_type, c_powerup_lifetime>(powerup_entity);

						pp.m_timers[pt.m_type] = globals::powerup_duration;
						pl.m_lifetime = 0.f;
					}

					void bullet_tile(entt::entity bullet_entity, entt::entity )
					{
						auto [bo, bl] = m_world.m_registry.get<c_bullet_owner_id, c_bullet_lifetime>(bullet_entity);
						auto const& opu = m_world.m_registry.get<c_player_powerups>(bo.m_owner_id);

						if (opu.m_timers.contains(powerup_type::bullet_bounce))
							return;
						
						bl.m_lifetime = 0.f;
					}

					void bullet_world_bounds(entt::entity bullet_entity)
					{
						auto [bo, bl] = m_world.m_registry.get<c_bullet_owner_id, c_bullet_lifetime>(bullet_entity);
						auto const& opu = m_world.m_registry.get<c_player_powerups>(bo.m_owner_id);

						if (opu.m_timers.contains(powerup_type::bullet_bounce))
							return;
						
						bl.m_lifetime = 0.f;
					}

					ta::world& m_world;
					ta::world_physics& m_world_physics;
				};

				auto cl = contact_listener(world, world_physics);
				world_physics.m_b2_world.SetContactListener(&cl);

				// update physics
				// TODO: use an accumulator to make this framerate independent
				world_physics.m_b2_world.Step(delta_time, 6, 2);

				// update bullet lifetimes
				{
					auto const bullet_view = world.m_registry.view<c_bullet_lifetime>();

					for (auto const b : bullet_view)
					{
						auto& bl = bullet_view.get<c_bullet_lifetime>(b);
						bl.m_lifetime -= delta_time;
					}
				}

				// remove expired bullets
				{
					auto const bullet_view = world.m_registry.view<c_bullet_lifetime, c_bullet_physics>();

					auto first_expired = std::partition(world.m_bullets.begin(), world.m_bullets.end(),
						[&] (auto const& b) { return bullet_view.get<c_bullet_lifetime>(b).m_lifetime > 0.f; });

					for (auto const b : std::ranges::subrange(first_expired, world.m_bullets.end()))
						destroy_bullet(world.m_registry, world_physics.m_b2_world, b);

					world.m_bullets.erase(first_expired, world.m_bullets.end());
				}

				// update player hp
				{
					auto const player_view = world.m_registry.view<c_player_hp, c_player_powerups>();

					for (auto const p : player_view)
					{
						auto [ph, pp] = player_view.get<c_player_hp, c_player_powerups>(p);

						auto hp_timer = pp.m_timers.find(powerup_type::player_heal);

						if (hp_timer == pp.m_timers.end())
							continue;

						ph.m_hp = std::min(ph.m_hp + globals::powerup_player_heal_hp, globals::player_hp);
						hp_timer->second = 0.f;
					}
				}

				// update player powerup timers
				{
					auto const player_view = world.m_registry.view<c_player_powerups>();

					for (auto const p : player_view)
					{
						auto& pp = player_view.get<c_player_powerups>(p);

						for (auto& [type, time] : pp.m_timers)
							time -= delta_time;

						std::erase_if(pp.m_timers,
							[] (auto const& p) { return p.second <= 0.f; });
					}
				}

				// update powerup lifetimes
				{
					auto const powerup_view = world.m_registry.view<c_powerup_lifetime>();

					for (auto const p : powerup_view)
					{
						auto& pl = powerup_view.get<c_powerup_lifetime>(p);
						pl.m_lifetime -= delta_time;
					}
				}

				// remove expired powerups
				{
					auto const powerup_view = world.m_registry.view<c_powerup_lifetime, c_powerup_physics>();

					auto first_expired = std::partition(world.m_powerups.begin(), world.m_powerups.end(),
						[&] (auto const& p) { return powerup_view.get<c_powerup_lifetime>(p).m_lifetime > 0.f; });

					for (auto const p : std::ranges::subrange(first_expired, world.m_powerups.end()))
						destroy_powerup(world.m_registry, world_physics.m_b2_world, p);

					world.m_powerups.erase(first_expired, world.m_powerups.end());
				}

				// remove dead players
				{
					auto const player_view = world.m_registry.view<c_player_hp, c_player_physics>();

					auto first_dead = std::partition(world.m_players.begin(), world.m_players.end(),
						[&] (auto const& p) { return player_view.get<c_player_hp>(p).m_hp > 0; });

					for (auto const p : std::ranges::subrange(first_dead, world.m_players.end()))
					{
						if (p == player_entity)
							player_entity = entt::null;

						destroy_player(world.m_registry, world_physics.m_b2_world, p);
					}

					world.m_players.erase(first_dead, world.m_players.end());
				}

				// spawn powerups
				{
					if (powerup_spawn_timer.get_elapsed_time() >= globals::powerup_spawn_time)
					{
						auto dist = std::uniform_int_distribution<std::size_t>{ 0, static_cast<std::size_t>(ta::powerup_type::COUNT) - 1 };
						auto const type = static_cast<ta::powerup_type>(dist(rng));

						auto const tile_view = world.m_registry.view<c_tile_physics>();

						tile_list.clear();
						tile_list.insert(tile_list.end(), tile_view.begin(), tile_view.end());

						auto const can_spawn = [&] (entt::entity tile)
						{
							auto tp = tile_view.get<c_tile_physics>(tile);

							auto const& fixture = *tp.m_b2_body->GetFixtureList();
							auto const& filter = fixture.GetFilterData();

							return (filter.categoryBits & collision_category::tile_wall) == 0
								&& (filter.categoryBits & collision_category::tile_void) == 0;
						};
						
						auto const end = std::remove_if(tile_list.begin(), tile_list.end(), std::not_fn(can_spawn));

						auto tile = entt::entity();
						std::sample(tile_list.begin(), end, &tile, 1, rng);
						
						auto const pos_px = globals::b2_inv_scale_factor * to_glm_vec2(tile_view.get<c_tile_physics>(tile).m_b2_body->GetPosition());

						world.m_powerups.push_back(create_powerup(world.m_registry, world_physics.m_b2_world, type, pos_px));
						powerup_spawn_timer = bump::timer();
					}
				}
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

				// render tiles
				{
					auto const tile_view = world.m_registry.view<c_tile_type>();

					for (auto y : bump::range(0, world.m_tiles.extents()[1]))
					{
						for (auto x : bump::range(0, world.m_tiles.extents()[0]))
						{
							auto const entity = world.m_tiles.at({ x, y });
							auto const& tt = tile_view.get<c_tile_type>(entity);

							auto const tile_index = static_cast<std::size_t>(tt.m_type);
							auto const tile_texture = world_graphics.m_tile_textures[tile_index];

							auto const position_px = globals::tile_radius + globals::tile_radius * 2.f * glm::vec2(x, y);

							auto model_matrix = glm::mat4(1.f);
							bump::set_position(model_matrix, glm::vec3(position_px, 0.f));

							world_graphics.m_tile_renderable.render(app.m_renderer, *tile_texture, matrices, model_matrix, globals::tile_radius * 2.f);
						}
					}
				}

				// render players
				{
					auto const player_view = world.m_registry.view<c_player_physics, c_player_movement, c_player_graphics>();

					for (auto const p : player_view)
					{
						auto [pp, pm, pg] = player_view.get<c_player_physics, c_player_movement, c_player_graphics>(p);

						auto const rotation_angle = dir_to_angle(pm.m_direction);
						auto const position_px = globals::b2_inv_scale_factor * to_glm_vec2(pp.m_b2_body->GetPosition());

						auto model_matrix = glm::mat4(1.f);
						bump::set_position(model_matrix, glm::vec3(position_px, 0.f));
						bump::set_rotation(model_matrix, glm::angleAxis(glm::radians(rotation_angle), glm::vec3(0.f, 0.f, 1.f)));

						auto& renderable = is_diagonal(pm.m_direction) ? world_graphics.m_tank_renderable_diagonal : world_graphics.m_tank_renderable;
						renderable.render(app.m_renderer, matrices, model_matrix, globals::player_radius * 2.f, pg.m_color);
					}
				}

				// render bullets
				{
					auto const player_color_view = world.m_registry.view<c_player_graphics>();
					auto const bullet_view = world.m_registry.view<c_bullet_owner_id, c_bullet_physics>();

					for (auto const b : bullet_view)
					{
						auto const& [bid, bp] = bullet_view.get<c_bullet_owner_id, c_bullet_physics>(b);
						auto const& pg = player_color_view.get<c_player_graphics>(bid.m_owner_id);

						auto const position_px = globals::b2_inv_scale_factor * to_glm_vec2(bp.m_b2_body->GetPosition());

						auto model_matrix = glm::mat4(1.f);
						bump::set_position(model_matrix, glm::vec3(position_px, 0.f));
						world_graphics.m_bullet_renderable.render(app.m_renderer, matrices, model_matrix, globals::bullet_radius * 2.f, pg.m_color);
					}
				}

				// render powerups
				{
					auto const powerup_view = world.m_registry.view<c_powerup_type, c_powerup_physics>();
					
					for (auto const p : powerup_view)
					{
						auto const& [pt, pp] = powerup_view.get<c_powerup_type, c_powerup_physics>(p);

						auto const position_px = globals::b2_inv_scale_factor * to_glm_vec2(pp.m_b2_body->GetPosition());
						auto const color = get_powerup_color(pt.m_type);

						auto model_matrix = glm::mat4(1.f);
						bump::set_position(model_matrix, glm::vec3(position_px, 0.f));
						world_graphics.m_powerup_renderable.render(app.m_renderer, matrices, model_matrix, globals::powerup_radius * 2.f, color);
					}
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

	// TODO:
		// better way of handling local player (maybe think about this more when separating client / server)
		// we want player objects to exist, even for dead / disconnected players

		// when doing bullet owner lookups, owning player might not exist anymore!!!
			// add a flag to the player to indicate if they are dead???
			// if they are, don't render / update / whatever...

	// ... finally ...
	// start working on server code!

// start working on the server code


	// namespace net
	// {
	//
	// 	enum class message_type : std::uint8_t { HELLO, GOODBYE, };
	//
	// 	using message_id_t = std::uint32_t;
	//
	// 	struct message
	// 	{
	// 		message_type m_type;
	// 		message_id_t m_id;
	// 		std::vector<std::uint8_t> m_data;
	// 	};
	//
	// } // net


	// using namespace bump;

	// auto const net_context = net::init_context().unwrap();
	// auto const local_endpoint = net::get_address_info_loopback(net::ip_address_family::V6, net::ip_protocol::UDP, 4377).unwrap().m_endpoints.front();
	// auto const remote_endpoint = net::get_address_info_loopback(net::ip_address_family::V6, net::ip_protocol::UDP, 4376).unwrap().m_endpoints.front();
	// auto udp_socket = net::make_udp_connected_socket(local_endpoint, remote_endpoint, net::blocking_mode::NON_BLOCKING).unwrap();
	// auto udp_read_buffer = std::vector<std::uint8_t>(128, '\0');

	// while (true)
	// {
	// 	// uh... yep!
	// }
