#pragma once

#include "ta_net_events.hpp"

#include <bump_enet.hpp>

#include <queue>

namespace ta::net
{

	class server
	{
	public:

		server() = default;
		explicit server(std::size_t peers, std::uint8_t channels, std::uint16_t port);

		server(server const&) = delete;
		server& operator=(server const&) = delete;
		server(server&&) = default;
		server& operator=(server&&) = default;

		~server();

		void poll(std::queue<net::peer_net_event>& net_events, std::queue<net::peer_game_event>& game_events);

		void broadcast(std::uint8_t channel_id, net::game_event event, int packet_flags);
		void send(bump::enet::peer peer, std::uint8_t channel_id, net::game_event event, int packet_flags);

		bump::enet::host m_host;
	};

} // ta::net
