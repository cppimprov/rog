#pragma once

namespace ta { struct world; namespace net { class server; namespace net_events { struct connect; struct disconnect; } } }

namespace ta
{

	// ta_server_player.cpp
	void spawn_player(ta::world& world, ta::net::net_events::connect& event, ta::net::server& server);
	void despawn_player(ta::world& world, ta::net::net_events::disconnect& event, ta::net::server& server);

} // ta
