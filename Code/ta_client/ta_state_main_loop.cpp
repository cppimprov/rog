#include "ta_state.hpp"

#include <ta_globals.hpp>
#include <ta_net_client.hpp>
#include <ta_world.hpp>

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_input.hpp>
#include <bump_log.hpp>
#include <bump_timer.hpp>

#include <entt.hpp>

#include <random>
#include <vector>

namespace ta
{

	bump::gamestate main_loop(bump::app& app, std::unique_ptr<ta::world> world_ptr, ta::net::client client)
	{
		bump::log_info("main_loop()");

		auto& world = *world_ptr;

		auto app_events = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();
		auto net_events = std::queue<ta::net::net_event>();
		auto game_events = std::queue<ta::net::game_event>();

		auto rng = std::mt19937_64{ std::random_device{}() };
		auto tile_list = std::vector<entt::entity>();

		auto timer = bump::frame_timer();
		auto frame_count = std::uint64_t{ 0 };
		auto dt_accumulator = bump::high_res_duration_t{ 0 };
		auto powerup_spawn_timer = bump::timer();
		
		auto const start_time = bump::high_res_clock_t::now();

		while (true)
		{
			dt_accumulator += timer.get_last_frame_time();

			while (dt_accumulator >= globals::client_update_tick_rate)
			{
				auto const frame_time = frame_count * globals::client_update_tick_rate;

				dt_accumulator -= globals::client_update_tick_rate;
				auto const dt_f = globals::client_update_tick_rate_f;

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

							// player input
							if (world.m_local_player != entt::null)
							{
								auto& pi = world.m_registry.get<c_player_input>(world.m_local_player);

								if (k.m_key == kt::W) pi.m_input_up = k.m_value;
								if (k.m_key == kt::S) pi.m_input_down = k.m_value;
								if (k.m_key == kt::A) pi.m_input_left = k.m_value;
								if (k.m_key == kt::D) pi.m_input_right = k.m_value;
								if (k.m_key == kt::SPACE) pi.m_input_fire = k.m_value;
							}
						}
					}
				}
				
				// network
				{
					client.poll(net_events, game_events);

					while (!net_events.empty())
					{
						auto event = std::move(net_events.front());
						net_events.pop();

						namespace ne = ta::net::net_events;

						if (std::holds_alternative<ne::connect>(event))
						{
							// note: this should never happen...
							bump::log_error("connected to server in main_loop!?");
							continue;
						}

						if (std::holds_alternative<ne::disconnect>(event))
						{
							bump::log_info("disconnected from server!");
							return { [&] (bump::app& app) { return loading(app); } };
						}
					}

					while (!game_events.empty())
					{
						bump::log_info("game event received!");

						auto event = std::move(game_events.front());
						game_events.pop();

						namespace ge = ta::net::game_events;

						// todo: ...
					}

				}

				// update
				{
					// send player input
					{
						if (world.m_local_player != entt::null)
						{
							auto [pm, pi] = world.m_registry.get<c_player_movement, c_player_input>(world.m_local_player);

							auto const dir = ta::get_input_dir(pi.m_input_up, pi.m_input_down, pi.m_input_left, pi.m_input_right);
							auto const moving = dir.has_value();
							auto const direction = dir.value_or(pm.m_direction);
							auto const firing = pi.m_input_fire;

							if (moving != pm.m_moving || direction != pm.m_direction || firing != pm.m_firing)
							{
								pm.m_moving = moving;
								pm.m_direction = direction;
								pm.m_firing = firing;

								client.send(0, net::game_events::input{ frame_time, moving, direction, firing }, ENET_PACKET_FLAG_RELIABLE);
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

						// todo: ???
					}

					// update physics
					{
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
						world.m_b2_world.Step(dt_f, 6, 2);

						// todo: ??? lerp / extrapolate ???
					}

					// update bullet lifetimes
					{
						auto const bullet_view = world.m_registry.view<c_bullet_lifetime>();

						for (auto const b : bullet_view)
						{
							auto& bl = bullet_view.get<c_bullet_lifetime>(b);
							bl.m_lifetime -= dt_f;
						}

						// todo: keep? use to set visibility on rendering only
					}

					// remove expired bullets
					{
						auto const bullet_view = world.m_registry.view<c_bullet_lifetime, c_bullet_physics>();

						auto first_expired = std::partition(world.m_bullets.begin(), world.m_bullets.end(),
							[&] (auto const& b) { return bullet_view.get<c_bullet_lifetime>(b).m_lifetime > 0.f; });

						for (auto const b : std::ranges::subrange(first_expired, world.m_bullets.end()))
							destroy_bullet(world.m_registry, world.m_b2_world, b);

						world.m_bullets.erase(first_expired, world.m_bullets.end());

						// todo: remove! (delete when the server tells us to)
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

						// todo: remove! (boost hp when the server tells us to)
					}

					// update player powerup timers
					{
						auto const player_view = world.m_registry.view<c_player_powerups>();

						for (auto const p : player_view)
						{
							auto& pp = player_view.get<c_player_powerups>(p);

							for (auto& [type, time] : pp.m_timers)
								time -= dt_f;

							std::erase_if(pp.m_timers,
								[] (auto const& p) { return p.second <= 0.f; });
						}

						// todo: keep time update! remove erasing expired timers (let server tell us when)!
					}

					// update powerup lifetimes
					{
						auto const powerup_view = world.m_registry.view<c_powerup_lifetime>();

						for (auto const p : powerup_view)
						{
							auto& pl = powerup_view.get<c_powerup_lifetime>(p);
							pl.m_lifetime -= dt_f;
						}

						// todo: keep!
					}

					// remove expired powerups
					{
						auto const powerup_view = world.m_registry.view<c_powerup_lifetime, c_powerup_physics>();

						auto first_expired = std::partition(world.m_powerups.begin(), world.m_powerups.end(),
							[&] (auto const& p) { return powerup_view.get<c_powerup_lifetime>(p).m_lifetime > 0.f; });

						for (auto const p : std::ranges::subrange(first_expired, world.m_powerups.end()))
							destroy_powerup(world.m_registry, world.m_b2_world, p);

						world.m_powerups.erase(first_expired, world.m_powerups.end());

						// todo: remove! (delete when the server tells us to)
					}

					// remove dead players
					{
						auto const player_view = world.m_registry.view<c_player_hp, c_player_physics>();

						auto first_dead = std::partition(world.m_players.begin(), world.m_players.end(),
							[&] (auto const& p) { return player_view.get<c_player_hp>(p).m_hp > 0; });

						for (auto const p : std::ranges::subrange(first_dead, world.m_players.end()))
						{
							destroy_player(world.m_registry, world.m_b2_world, p);

							auto slot = std::find_if(world.m_player_slots.begin(), world.m_player_slots.end(),
								[&] (auto const& s) { return s.m_entity == p; });
							
							if (slot == world.m_player_slots.end())
							{
								bump::log_error("player slot not found!");
								continue;
							}

							slot->m_entity = entt::null;

							if (world.m_local_player == p)
								world.m_local_player = entt::null;
						}

						world.m_players.erase(first_dead, world.m_players.end());

						// todo: remove! (delete when the server tells us to)
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

						// todo: remove! (spawn when the server tells us to)
					}
				}

				++frame_count;
			}

			render_world(app.m_window, app.m_renderer, world);

			timer.tick();
		}
	}

} // ta
