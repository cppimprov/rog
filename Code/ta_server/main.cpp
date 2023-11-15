
#include "ta_world.hpp"
#include "ta_globals.hpp"

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_io.hpp>
#include <bump_log.hpp>
#include <bump_net.hpp>
#include <bump_transform.hpp>

#include <enet/enet.h>

#include <algorithm>
#include <random>

namespace ta
{

	enum class net_event_type : std::uint8_t { SPAWN, DESPAWN, READY, };

	using host_ptr = std::unique_ptr<ENetHost, decltype(&enet_host_destroy)>;

	bump::gamestate main_loop(bump::app& app, std::unique_ptr<ta::world> world_ptr, host_ptr server)
	{
		bump::log_info("main_loop()");

		auto& world = *world_ptr;

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
					}
				}
			}
			
			// update
			{
				auto const delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(timer.get_last_frame_time()).count();

				// update player input
				{
					// ...
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
					contact_listener(entt::registry& registry, b2World& b2_world):
						m_registry(registry),
						m_b2_world(b2_world)
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
						auto& ph = m_registry.get<c_player_hp>(player_entity);
						auto [bo, bl] = m_registry.get<c_bullet_owner_id, c_bullet_lifetime>(bullet_entity);

						if (bo.m_owner_id == player_entity)
							return;

						auto const& opu = m_registry.get<c_player_powerups>(bo.m_owner_id);
						auto const damage_multiplier = opu.m_timers.contains(powerup_type::bullet_damage) ? globals::powerup_bullet_damage_multiplier : 1.f;

						ph.m_hp -= static_cast<std::uint32_t>(globals::bullet_damage * damage_multiplier);
						bl.m_lifetime = 0.f;
					}

					void player_powerup(entt::entity player_entity, entt::entity powerup_entity)
					{
						auto& pp = m_registry.get<c_player_powerups>(player_entity);
						auto [pt, pl] = m_registry.get<c_powerup_type, c_powerup_lifetime>(powerup_entity);

						pp.m_timers[pt.m_type] = globals::powerup_duration;
						pl.m_lifetime = 0.f;
					}

					void bullet_tile(entt::entity bullet_entity, entt::entity )
					{
						auto [bo, bl] = m_registry.get<c_bullet_owner_id, c_bullet_lifetime>(bullet_entity);
						auto const& opu = m_registry.get<c_player_powerups>(bo.m_owner_id);

						if (opu.m_timers.contains(powerup_type::bullet_bounce))
							return;
						
						bl.m_lifetime = 0.f;
					}

					void bullet_world_bounds(entt::entity bullet_entity)
					{
						auto [bo, bl] = m_registry.get<c_bullet_owner_id, c_bullet_lifetime>(bullet_entity);
						auto const& opu = m_registry.get<c_player_powerups>(bo.m_owner_id);

						if (opu.m_timers.contains(powerup_type::bullet_bounce))
							return;
						
						bl.m_lifetime = 0.f;
					}

					entt::registry& m_registry;
					b2World& m_b2_world;
				};

				auto cl = contact_listener(world.m_registry, world.m_b2_world);
				world.m_b2_world.SetContactListener(&cl);

				// update physics
				// TODO: use an accumulator to make this framerate independent
				world.m_b2_world.Step(delta_time, 6, 2);

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
						destroy_bullet(world.m_registry, world.m_b2_world, b);

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
						destroy_powerup(world.m_registry, world.m_b2_world, p);

					world.m_powerups.erase(first_expired, world.m_powerups.end());
				}

				// remove dead players
				{
					auto const player_view = world.m_registry.view<c_player_hp, c_player_physics>();

					auto first_dead = std::partition(world.m_players.begin(), world.m_players.end(),
						[&] (auto const& p) { return player_view.get<c_player_hp>(p).m_hp > 0; });

					for (auto const p : std::ranges::subrange(first_dead, world.m_players.end()))
						destroy_player(world.m_registry, world.m_b2_world, p);

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

						world.m_powerups.push_back(create_powerup(world.m_registry, world.m_b2_world, type, pos_px));
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
							auto const tile_texture = world.m_tile_textures[tile_index];

							auto const position_px = globals::tile_radius + globals::tile_radius * 2.f * glm::vec2(x, y);

							auto model_matrix = glm::mat4(1.f);
							bump::set_position(model_matrix, glm::vec3(position_px, 0.f));

							world.m_tile_renderable.render(app.m_renderer, *tile_texture, matrices, model_matrix, globals::tile_radius * 2.f);
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

						auto& renderable = is_diagonal(pm.m_direction) ? world.m_tank_renderable_diagonal : world.m_tank_renderable;
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
						world.m_bullet_renderable.render(app.m_renderer, matrices, model_matrix, globals::bullet_radius * 2.f, pg.m_color);
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
						world.m_powerup_renderable.render(app.m_renderer, matrices, model_matrix, globals::powerup_radius * 2.f, color);
					}
				}
				
				app.m_renderer.set_blending(bump::gl::renderer::blending::NONE);

				app.m_window.swap_buffers();
			}

			timer.tick();
		}
	}

	bump::gamestate waiting_for_players(bump::app& app, std::unique_ptr<ta::world> world_ptr)
	{
		auto& world = *world_ptr;

		auto app_events = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();

		auto const e_port = std::uint16_t{ 6543 };
		auto const e_addr = ENetAddress{ ENET_HOST_ANY, e_port };
		auto server = host_ptr(enet_host_create(&e_addr, 4, 2, 0, 0), &enet_host_destroy);

		if (!server)
		{
			bump::log_error("failed to create enet server!");
			return { };
		}

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
					}
				}
			}

			// network
			{
				auto event = ENetEvent{ };

				while (enet_host_service(server.get(), &event, 0) > 0)
				{
					switch (event.type)
					{
					case ENET_EVENT_TYPE_CONNECT:
					{
						bump::log_info("client connected!");

						// find an empty player slot
						auto const new_slot = std::find_if(world.m_player_slots.begin(), world.m_player_slots.end(),
							[] (auto const& s) { return s.m_entity == entt::null; });

						if (new_slot == world.m_player_slots.end())
						{
							bump::log_info("no player slots available!");
							enet_peer_disconnect_now(event.peer, 0);
							break;
						}

						// create player 
						world.m_players.push_back(create_player(world.m_registry, world.m_b2_world, new_slot->m_start_pos_px, new_slot->m_color));

						// occupy player slot
						new_slot->m_entity = world.m_players.back();
						new_slot->m_peer = event.peer;

						// send spawn event to everyone
						auto const new_entity = world.m_players.back();
						auto const new_slot_index = new_slot - world.m_player_slots.begin();

						{
							auto stream = std::ostringstream();
							bump::io::write(stream, static_cast<std::uint8_t>(net_event_type::SPAWN));
							bump::io::write(stream, static_cast<std::uint8_t>(new_slot_index));
							bump::io::write(stream, true);

							auto const& str = stream.str();
							auto const packet = enet_packet_create(str.data(), str.size(), ENET_PACKET_FLAG_RELIABLE);

							enet_host_broadcast(server.get(), 0, packet);
						}
						
						// update new client by spawning other players
						for (auto slot_index = std::size_t{ 0 }; slot_index != world.m_player_slots.size(); ++slot_index)
						{
							auto const& slot = world.m_player_slots[slot_index];

							if (slot.m_entity == entt::null || slot.m_entity == new_entity)
								continue;

							auto stream = std::ostringstream();
							bump::io::write(stream, static_cast<std::uint8_t>(net_event_type::SPAWN));
							bump::io::write(stream, static_cast<std::uint8_t>(slot_index));
							bump::io::write(stream, false);

							auto const& str = stream.str();
							auto const packet = enet_packet_create(str.data(), str.size(), ENET_PACKET_FLAG_RELIABLE);

							enet_peer_send(event.peer, 0, packet);
						}

						break;
					}
					case ENET_EVENT_TYPE_RECEIVE:
					{
						bump::log_info("packet received!");

						// note: server shouldn't be receiving any packets here!

						enet_packet_destroy(event.packet);

						break;
					}
					case ENET_EVENT_TYPE_DISCONNECT:
					{
						bump::log_info("client disconnected!");

						// find corresponding player slot
						auto const slot = std::find_if(world.m_player_slots.begin(), world.m_player_slots.end(),
							[&] (auto const& s) { return s.m_peer == event.peer; });
						
						if (slot == world.m_player_slots.end())
						{
							bump::log_info("client not found!");
							break;
						}

						// send despawn event to everyone
						auto const entity = slot->m_entity;
						auto const slot_index = slot - world.m_player_slots.begin();

						auto stream = std::ostringstream();
						bump::io::write(stream, static_cast<std::uint8_t>(net_event_type::DESPAWN));
						bump::io::write(stream, static_cast<std::uint8_t>(slot_index));

						auto const& str = stream.str();
						auto const packet = enet_packet_create(str.data(), str.size(), ENET_PACKET_FLAG_RELIABLE);

						enet_host_broadcast(server.get(), 0, packet);

						// remove player's bullets from registry
						auto const bullet_view = world.m_registry.view<c_bullet_owner_id>();

						auto expired = std::partition(world.m_bullets.begin(), world.m_bullets.end(),
							[&] (auto const& b) { return bullet_view.get<c_bullet_owner_id>(b).m_owner_id != entity; });

						for (auto const b : std::ranges::subrange(expired, world.m_bullets.end()))
							destroy_bullet(world.m_registry, world.m_b2_world, b);

						// remove player's bullets from world
						world.m_bullets.erase(expired, world.m_bullets.end());

						// remove player from registry
						destroy_player(world.m_registry, world.m_b2_world, entity);

						// clear player slot
						slot->m_entity = entt::null;
						slot->m_peer = nullptr;

						break;
					}
					}
				}

				// notify players to move to the main_loop
				if (server->connectedPeers == 4)
				{
					auto stream = std::ostringstream();
					bump::io::write(stream, static_cast<std::uint8_t>(net_event_type::READY));

					auto const& str = stream.str();
					auto const packet = enet_packet_create(str.data(), str.size(), ENET_PACKET_FLAG_RELIABLE);

					enet_host_broadcast(server.get(), 0, packet);

					return { [&, world = std::move(world_ptr), server = std::move(server)] (bump::app& app) mutable { return main_loop(app, std::move(world), std::move(server)); } };
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
							auto const tile_texture = world.m_tile_textures[tile_index];

							auto const position_px = globals::tile_radius + globals::tile_radius * 2.f * glm::vec2(x, y);

							auto model_matrix = glm::mat4(1.f);
							bump::set_position(model_matrix, glm::vec3(position_px, 0.f));

							world.m_tile_renderable.render(app.m_renderer, *tile_texture, matrices, model_matrix, globals::tile_radius * 2.f);
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

						auto& renderable = is_diagonal(pm.m_direction) ? world.m_tank_renderable_diagonal : world.m_tank_renderable;
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
						world.m_bullet_renderable.render(app.m_renderer, matrices, model_matrix, globals::bullet_radius * 2.f, pg.m_color);
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
						world.m_powerup_renderable.render(app.m_renderer, matrices, model_matrix, globals::powerup_radius * 2.f, color);
					}
				}
				
				app.m_renderer.set_blending(bump::gl::renderer::blending::NONE);

				app.m_window.swap_buffers();
			}
		}

		return { };
	}

	bump::gamestate loading(bump::app& app)
	{
		bump::log_info("loading()");

		auto world = std::unique_ptr<ta::world>(new ta::world
		{
			.m_player_slots =
			{
				{ glm::vec3(1.f, 0.8f, 0.3f), globals::player_radius * glm::vec2{ 1.f, 8.f }, entt::null, nullptr },
				{ glm::vec3(1.f, 0.f, 0.f), globals::player_radius * glm::vec2{ 5.f, 3.f }, entt::null, nullptr },
				{ glm::vec3(0.f, 0.9f, 0.f), globals::player_radius * glm::vec2{ 7.f, 3.f }, entt::null, nullptr },
				{ glm::vec3(0.2f, 0.2f, 1.f), globals::player_radius * glm::vec2{ 9.f, 3.f }, entt::null, nullptr },
			},

			.m_b2_world = b2World{ b2Vec2{ 0.f, 0.f } },

			.m_tile_textures = // TODO: these are map specific? should be done in load_test_map()?
			{ 
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
		});

		load_test_map(*world);
		set_world_bounds(world->m_b2_world, glm::vec2(world->m_tiles.extents()) * globals::tile_radius * 2.f);

		return { [&, world = std::move(world)] (bump::app& app) mutable { return waiting_for_players(app, std::move(world)); } };
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

		// temp:
		if (enet_initialize() != 0)
		{
			bump::log_error("failed to initialize enet!");
			return EXIT_FAILURE;
		}

		bump::run_state({ [] (bump::app& app) { return ta::loading(app); } }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}

// TODO:

	// todo: server / client classes for basic raii (call disconnect_now on quit)
	// todo: share packet reading / writing code between server and client
	// todo: share spawn / despawn functions between server and client
