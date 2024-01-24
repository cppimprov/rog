#pragma once

#include "bump_enet_address.hpp"
#include "bump_enet_event.hpp"
#include "bump_enet_peer.hpp"

#include <enet/enet.h>

#include <optional>

namespace bump
{

	namespace enet
	{

		class host
		{
		public:

			host() = default;
			explicit host(ENetHost* host);
			explicit host(address addr, std::size_t peers);
			explicit host(address addr, std::size_t peers, std::uint8_t channels, std::uint32_t bandwidth_in, std::uint32_t bandwidth_out);
			explicit host(std::size_t peers, std::uint8_t channels, std::uint32_t bandwidth_in, std::uint32_t bandwidth_out);

			host(host const&) = delete;
			host& operator=(host const&) = delete;
			host(host&&);
			host& operator=(host&&);

			~host();

			address get_address() const { die_if(!m_host); return address(m_host->address); }

			std::size_t get_peer_count() const { die_if(!m_host); return m_host->peerCount; }
			std::size_t get_connected_peer_count() const { die_if(!m_host); return m_host->connectedPeers; }

			peer get_peer(std::size_t index) const { die_if(!m_host); return peer(&m_host->peers[index]); }
			
			std::uint32_t get_total_data_sent() const { die_if(!m_host); return m_host->totalSentData; }
			std::uint32_t get_total_data_received() const { die_if(!m_host); return m_host->totalReceivedData; }
			std::uint32_t get_total_packets_sent() const { die_if(!m_host); return m_host->totalSentPackets; }
			std::uint32_t get_total_packets_received() const { die_if(!m_host); return m_host->totalReceivedPackets; }

			void reset_totals();

			std::uint8_t get_channel_limit() const { die_if(!m_host); return static_cast<std::uint8_t>(m_host->channelLimit); }
			void set_channel_limit(std::uint8_t limit) { die_if(!m_host); enet_host_channel_limit(m_host, limit); }

			std::uint32_t get_incoming_bandwidth_limit() const { die_if(!m_host); return m_host->incomingBandwidth; }
			void set_incoming_bandwidth_limit(std::uint32_t limit) { die_if(!m_host); enet_host_bandwidth_limit(m_host, limit, m_host->outgoingBandwidth); }

			std::uint32_t get_outgoing_bandwidth_limit() const { die_if(!m_host); return m_host->outgoingBandwidth; }
			void set_outgoing_bandwidth_limit(std::uint32_t limit) { die_if(!m_host); enet_host_bandwidth_limit(m_host, m_host->incomingBandwidth, limit); }

			peer connect(address addr, std::size_t channels, std::uint32_t user_data);
			
			event check_events();
			event service(std::uint32_t timeout_ms);
			
			void broadcast(std::uint8_t channel_id, packet packet);
			void flush();

			bool is_valid() const { return m_host != nullptr; }
			void destroy();

			ENetHost* get_enet_host() const { return m_host; }

		private:

			ENetHost* m_host;
		};
	
	} // enet

} // bump
