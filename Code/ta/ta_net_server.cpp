#include "ta_net_server.hpp"

#include "bump_log.hpp"

#include <spanstream>
#include <sstream>

namespace ta::net
{
	
	server::server(std::size_t peers, std::uint8_t channels, std::uint16_t port):
		m_host({ 0u, port }, peers, channels, 0u, 0u) { }

	server::~server()
	{
		if (!m_host.is_valid())
			return;

		for (auto p = m_host.peer_begin(); p != m_host.peer_end(); ++p)
		{
			if (!p->is_valid())
				continue;

			p->disconnect_now(0);
		}
	}

	void server::poll(std::queue<net::net_event>& net_events, std::queue<net::game_event>& game_events)
	{
		while (true)
		{

			auto event = m_host.service(0);

			if (!event.is_valid())
				break;
			
			switch (event.get_type())
			{

			case bump::enet::event::type::connect:
			{
				net_events.push({ net_events::connect{ event.get_peer() } });
				break;
			}

			case bump::enet::event::type::receive:
			{
				auto const packet = event.take_packet();

				if (!packet.is_valid() || packet.size() == 0)
				{
					bump::log_error("empty packet received!");
					break;
				}

				auto const span = std::span(reinterpret_cast<char const*>(packet.data()), packet.size());
				auto packet_stream = std::ispanstream(span);
				auto const packet_event = bump::io::read<net::game_event>(packet_stream);

				if (!packet_stream.good())
				{
					bump::log_error("failed to read packet!");
					break;
				}

				if (std::holds_alternative<std::monostate>(packet_event))
				{
					bump::log_error("invalid packet received!");
					break;
				}

				game_events.push(packet_event);
				break;
			}

			case bump::enet::event::type::disconnect:
			{
				net_events.push({ net_events::disconnect{ event.get_peer() } });
				break;
			}
			
			}
		}
	}

	void server::broadcast(std::uint8_t channel_id, net::game_event event, int packet_flags)
	{
		auto stream = std::ostringstream();
		bump::io::write(stream, event);
		m_host.broadcast(channel_id, bump::enet::packet(stream.str(), packet_flags));
	}

	void server::send(bump::enet::peer peer, std::uint8_t channel_id, net::game_event event, int packet_flags)
	{
		auto stream = std::ostringstream();
		bump::io::write(stream, event);
		peer.send(channel_id, bump::enet::packet(stream.str(), packet_flags));
	}

} // ta::net
