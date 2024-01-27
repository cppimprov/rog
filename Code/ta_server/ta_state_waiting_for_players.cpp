#include "ta_state.hpp"

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_input.hpp>

#include <ta_net_server.hpp>
#include <ta_world.hpp>

namespace ta
{

	namespace
	{

		void spawn_player(ta::world& world, ta::net::net_events::connect& event, ta::net::server& server)
		{
			// find an empty player slot
			auto const new_slot = std::find_if(world.m_player_slots.begin(), world.m_player_slots.end(),
				[] (auto const& s) { return s.m_entity == entt::null; });

			if (new_slot == world.m_player_slots.end())
			{
				bump::log_info("no player slots available!");
				event.m_peer.disconnect_now(0);
				return;
			}

			// create player 
			world.m_players.push_back(create_player(world.m_registry, world.m_b2_world, new_slot->m_start_pos_px, new_slot->m_color));

			// occupy player slot
			new_slot->m_entity = world.m_players.back();
			new_slot->m_peer = event.m_peer;

			// send spawn event to everyone
			auto const new_entity = world.m_players.back();
			auto const new_slot_index = static_cast<std::uint8_t>(new_slot - world.m_player_slots.begin());
			server.broadcast(0, net::game_events::spawn{ new_slot_index, true }, ENET_PACKET_FLAG_RELIABLE);
			
			// update new client by spawning other players
			for (auto slot_index = std::size_t{ 0 }; slot_index != world.m_player_slots.size(); ++slot_index)
			{
				auto const& slot = world.m_player_slots[slot_index];

				if (slot.m_entity == entt::null || slot.m_entity == new_entity)
					continue;
				
				server.send(event.m_peer, 0, net::game_events::spawn{ static_cast<std::uint8_t>(slot_index), false }, ENET_PACKET_FLAG_RELIABLE);
			}
		}

		void despawn_player(ta::world& world, ta::net::net_events::disconnect& event, ta::net::server& server)
		{
			// find corresponding player slot
			auto const slot = std::find_if(world.m_player_slots.begin(), world.m_player_slots.end(),
				[&] (auto const& s) { return s.m_peer == event.m_peer; });
			
			if (slot == world.m_player_slots.end())
			{
				bump::log_info("client not found!");
				return;
			}

			// send despawn event to everyone
			auto const entity = slot->m_entity;
			auto const slot_index = slot - world.m_player_slots.begin();
			server.broadcast(0, net::game_events::despawn{ static_cast<std::uint8_t>(slot_index) }, ENET_PACKET_FLAG_RELIABLE);

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
			slot->m_peer = bump::enet::peer();
		}

	} // unnamed

	bump::gamestate waiting_for_players(bump::app& app, std::unique_ptr<ta::world> world_ptr)
	{
		auto& world = *world_ptr;

		auto app_events = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();
		auto net_events = std::queue<ta::net::net_event>();
		auto game_events = std::queue<ta::net::game_event>();

		auto server = ta::net::server(4, 2, 6543);

		if (!server.m_host.is_valid())
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
				server.poll(net_events, game_events);

				while (!net_events.empty())
				{
					auto event = std::move(net_events.front());
					net_events.pop();

					namespace ne = ta::net::net_events;

					if (std::holds_alternative<ne::connect>(event))
					{
						bump::log_info("client connected!");
						spawn_player(world, std::get<ne::connect>(event), server);
						continue;
					}

					if (std::holds_alternative<ne::disconnect>(event))
					{
						bump::log_info("client disconnected!");
						despawn_player(world, std::get<ne::disconnect>(event), server);
						continue;
					}
				}

				while (!game_events.empty())
				{
					bump::log_info("game event received!");

					// note: we shouldn't be receiving any game events here!
				}

				// notify players to move to the main_loop
				if (server.m_host.get_connected_peer_count() == 4)
				{
					server.broadcast(0, net::game_events::ready{ }, ENET_PACKET_FLAG_RELIABLE);

					return { [&, world = std::move(world_ptr), server = std::move(server)] (bump::app& app) mutable { return main_loop(app, std::move(world), std::move(server)); } };
				}
			}

			render_world(app.m_window, app.m_renderer, world);
		}

		return { };
	}

} // ta