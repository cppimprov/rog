#pragma once

#include "ta_net_events.hpp"

#include <bump_enet.hpp>

#include <queue>

namespace ta::net
{

	class client
	{
	public:

		client() = default;
		explicit client(std::uint8_t channels, bump::enet::address const& address);

		client(client const&) = delete;
		client& operator=(client const&) = delete;
		client(client&&) = default;
		client& operator=(client&&) = default;

		~client();

		void poll(std::queue<net::net_event>& net_events, std::queue<net::game_event>& game_events);

		void send(std::uint8_t channel_id, net::game_event event, int packet_flags);

		bump::enet::host m_host; // local endpoint
		bump::enet::peer m_peer; // remote (server) connection
	};

} // ta::net