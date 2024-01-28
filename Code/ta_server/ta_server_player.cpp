#include "ta_server.hpp"

#include <ta_net_server.hpp>
#include <ta_world.hpp>

#include <bump_log.hpp>

#include <algorithm>

namespace ta
{

	void spawn_player(ta::world& world, ta::net::server& server, bump::enet::peer peer)
	{
		// find an empty player slot
		auto const new_slot = std::find_if(world.m_player_slots.begin(), world.m_player_slots.end(),
			[] (auto const& s) { return s.m_entity == entt::null; });

		if (new_slot == world.m_player_slots.end())
		{
			bump::log_info("no player slots available!");
			peer.disconnect_now(0);
			return;
		}

		// create player 
		world.m_players.push_back(create_player(world.m_registry, world.m_b2_world, new_slot->m_start_pos_px, new_slot->m_color));

		// occupy player slot
		new_slot->m_entity = world.m_players.back();
		new_slot->m_peer = peer;

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
			
			server.send(peer, 0, net::game_events::spawn{ static_cast<std::uint8_t>(slot_index), false }, ENET_PACKET_FLAG_RELIABLE);
		}
	}

	void despawn_player(ta::world& world, ta::net::server& server, bump::enet::peer peer)
	{
		// find corresponding player slot
		auto const slot = std::find_if(world.m_player_slots.begin(), world.m_player_slots.end(),
			[&] (auto const& s) { return s.m_peer == peer; });
		
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

		// remove player from world
		std::erase(world.m_players, entity);

		// clear player slot
		slot->m_entity = entt::null;
		slot->m_peer = bump::enet::peer();
	}

} // ta
