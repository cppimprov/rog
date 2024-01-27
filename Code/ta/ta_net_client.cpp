#include "ta_net_client.hpp"

#include "bump_log.hpp"

#include <spanstream>
#include <sstream>

namespace ta::net
{

	client::client(std::uint8_t channels, bump::enet::address const& address):
		m_host(1, channels, 0, 0),
		m_peer(m_host.connect(address, channels, 0))
	{

	}

	client::~client()
	{
		if (m_peer.is_valid())
			m_peer.disconnect_now(0);
	}

	void client::poll(std::queue<net::net_event>& net_events, std::queue<net::game_event>& game_events)
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
					bump::log_error("invalid packet!");
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

	void client::send(std::uint8_t channel_id, net::game_event event, int packet_flags)
	{
		auto stream = std::ostringstream();
		bump::io::write(stream, event);
		m_peer.send(channel_id, bump::enet::packet(stream.str(), packet_flags));
	}

} // ta::net
