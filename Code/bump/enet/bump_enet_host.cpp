#include "bump_enet_host.hpp"

#include "bump_die.hpp"

namespace bump
{

	namespace enet
	{
	
		host::host(ENetHost* host):
			m_host(host) { }

		host::host(address addr, std::size_t peers):
			m_host(enet_host_create(addr.get_enet_address(), peers, 0, 0, 0)) { }

		host::host(address addr, std::size_t peers, std::uint8_t channels, std::uint32_t bandwidth_in, std::uint32_t bandwidth_out):
			m_host(enet_host_create(addr.get_enet_address(), peers, channels, bandwidth_in, bandwidth_out)) { }

		host::host(std::size_t peers, std::uint8_t channels, std::uint32_t bandwidth_in, std::uint32_t bandwidth_out):
			m_host(enet_host_create(nullptr, peers, channels, bandwidth_in, bandwidth_out)) { }
		
		host::host(host&& other):
			m_host(other.m_host)
		{
			other.m_host = nullptr;
		}

		host& host::operator=(host&& other)
		{
			auto temp = std::move(other);
			std::swap(m_host, temp.m_host);
			return *this;
		}

		host::~host()
		{
			destroy();
		}

		peer const* host::peer_begin() const
		{
			die_if(!m_host);
			return reinterpret_cast<peer const*>(m_host->peers);
		}

		peer* host::peer_begin()
		{
			die_if(!m_host);
			return reinterpret_cast<peer*>(m_host->peers);
		}

		peer const* host::peer_end() const
		{
			die_if(!m_host);
			return reinterpret_cast<peer const*>(m_host->peers + m_host->peerCount);
		}

		peer* host::peer_end()
		{
			die_if(!m_host);
			return reinterpret_cast<peer*>(m_host->peers + m_host->peerCount);
		}

		void host::reset_totals()
		{
			die_if(!m_host);
			m_host->totalSentData = 0;
			m_host->totalReceivedData = 0;
			m_host->totalSentPackets = 0;
			m_host->totalReceivedPackets = 0;
		}

		peer host::connect(address addr, std::size_t channels, std::uint32_t user_data)
		{
			die_if(!m_host);
			return peer(enet_host_connect(m_host, addr.get_enet_address(), channels, user_data));
		}

		event host::check_events()
		{
			die_if(!m_host);

			auto e = ENetEvent{ ENET_EVENT_TYPE_NONE, nullptr, 0, 0, nullptr };
			enet_host_check_events(m_host, &e);

			return event(e);
		}

		event host::service(std::uint32_t timeout_ms)
		{
			die_if(!m_host);

			auto e = ENetEvent{ ENET_EVENT_TYPE_NONE, nullptr, 0, 0, nullptr };
			enet_host_service(m_host, &e, timeout_ms);

			return event(e);
		}

		void host::broadcast(std::uint8_t channel_id, packet packet)
		{
			die_if(!m_host);
			enet_host_broadcast(m_host, channel_id, packet.release());
		}

		void host::flush()
		{
			die_if(!m_host);
			enet_host_flush(m_host);
		}

		void host::destroy()
		{
			enet_host_destroy(m_host);
			m_host = nullptr;
		}

	} // enet

} // bump
