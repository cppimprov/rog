#pragma once

#include <ta_net_server.hpp>

namespace ta { struct world; }

namespace ta
{

	// ta_server_player.cpp
	void spawn_player(ta::world& world, ta::net::server& server, bump::enet::peer peer);
	void despawn_player(ta::world& world, ta::net::server& server, bump::enet::peer peer);

} // ta
