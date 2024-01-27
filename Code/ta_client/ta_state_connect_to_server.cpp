#include "ta_state.hpp"

#include <ta_net_client.hpp>
#include <ta_world.hpp>

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_input.hpp>
#include <bump_log.hpp>

namespace ta
{

	bump::gamestate connect_to_server(bump::app& app, std::unique_ptr<ta::world> world_ptr)
	{
		auto& world = *world_ptr;

		auto app_events = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();
		auto net_events = std::queue<ta::net::net_event>();
		auto game_events = std::queue<ta::net::game_event>();

		auto const server_address = bump::enet::address("localhost", 6543u);
		auto client = ta::net::client(2, server_address);

		if (!client.m_host.is_valid())
		{
			bump::log_error("failed to create enet client!");
			return { };
		}

		if (!client.m_peer.is_valid())
		{
			bump::log_error("failed to connect to server!");
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
				client.poll(net_events, game_events);

				while (!net_events.empty())
				{
					auto event = std::move(net_events.front());
					net_events.pop();

					namespace ne = ta::net::net_events;

					if (std::holds_alternative<ne::connect>(event))
					{
						bump::log_info("connected to server!");
						continue;
					}

					if (std::holds_alternative<ne::disconnect>(event))
					{
						bump::log_info("disconnected from server!");
						return { };
					}
				}

				while (!game_events.empty())
				{
					auto event = std::move(game_events.front());
					game_events.pop();

					namespace ge = ta::net::game_events;

					if (std::holds_alternative<ge::spawn>(event))
					{
						bump::log_info("received spawn event!");
						
						auto const& e = std::get<ge::spawn>(event);

						if (e.m_slot_index >= world.m_player_slots.size())
						{
							bump::log_error("invalid slot index in spawn packet!");
							break;
						}

						// find the specified slot
						auto& slot = world.m_player_slots[e.m_slot_index];

						if (slot.m_entity != entt::null)
						{
							bump::log_error("slot already occupied!");
							break;
						}

						// create player
						world.m_players.push_back(create_player(world.m_registry, world.m_b2_world, slot.m_start_pos_px, slot.m_color));

						// spawn input component for local player
						if (e.m_self)
							world.m_registry.emplace<c_player_input>(world.m_players.back());

						// occupy player slot
						slot.m_entity = world.m_players.back();

						continue;
					}

					if (std::holds_alternative<ge::despawn>(event))
					{
						bump::log_info("received despawn event!");
						
						auto const& e = std::get<ge::despawn>(event);

						if (e.m_slot_index >= world.m_player_slots.size())
						{
							bump::log_error("invalid slot index in despawn packet!");
							break;
						}

						// find the player slot
						auto& slot = world.m_player_slots[e.m_slot_index];

						auto const entity = slot.m_entity;

						if (entity == entt::null)
						{
							bump::log_error("slot already empty!");
							break;
						}

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
						slot.m_entity = entt::null;
						slot.m_peer = bump::enet::peer();

						continue;
					}

					if (std::holds_alternative<ge::ready>(event))
					{
						bump::log_info("received ready event!");

						return { [&, world = std::move(world_ptr), client = std::move(client)] (bump::app& app) mutable { return main_loop(app, std::move(world), std::move(client)); } };
					}
				}
			}

			render_world(app.m_window, app.m_renderer, world);
		}

		return { };
	}
	
} // ta
