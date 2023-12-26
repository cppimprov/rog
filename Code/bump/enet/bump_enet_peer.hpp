#pragma once

#include "bump_die.hpp"
#include "bump_enet_address.hpp"
#include "bump_enet_packet.hpp"

#include <enet/enet.h>

#include <cstdint>
#include <optional>

namespace bump
{

	namespace enet
	{

		class peer
		{
		public:

			peer(): m_peer(nullptr) { }
			explicit peer(ENetPeer* peer): m_peer(peer) { }
			
			peer(peer const&) = default;
			peer& operator=(peer const&) = default;
			peer(peer&&) = default;
			peer& operator=(peer&&) = default;

			address get_address() const { die_if(!m_peer); return address(m_peer->address); }

			std::uint8_t get_channel_count() const { die_if(!m_peer); return static_cast<std::uint8_t>(m_peer->channelCount); }

			void send(std::uint8_t channel_id, packet packet);
			packet receive(std::uint8_t& channel_id);

			void set_ping_interval(std::uint32_t interval_ms) { die_if(!m_peer); enet_peer_ping_interval(m_peer, interval_ms); }
			void ping() { die_if(!m_peer); enet_peer_ping(m_peer); }

			void set_timeout(std::uint32_t limit_ms, std::uint32_t minimum_ms, std::uint32_t maximum_ms) { die_if(!m_peer); enet_peer_timeout(m_peer, limit_ms, minimum_ms, maximum_ms); }

			void disconnect(std::uint32_t data) { die_if(!m_peer); enet_peer_disconnect(m_peer, data); }
			void disconnect_now(std::uint32_t data) { die_if(!m_peer); enet_peer_disconnect_now(m_peer, data); }
			void disconnect_later(std::uint32_t data) { die_if(!m_peer); enet_peer_disconnect_later(m_peer, data); }
			void reset() { die_if(!m_peer); enet_peer_reset(m_peer); }

			std::uint32_t get_bandwidth_incoming() const { die_if(!m_peer); return m_peer->incomingBandwidth; }
			std::uint32_t get_bandwidth_outgoing() const { die_if(!m_peer); return m_peer->outgoingBandwidth; }
			std::uint32_t get_total_data_sent() const { die_if(!m_peer); return m_peer->outgoingDataTotal; }
			std::uint32_t get_total_data_received() const { die_if(!m_peer); return m_peer->incomingDataTotal; }
			std::uint32_t get_packets_sent() const { die_if(!m_peer); return m_peer->packetsSent; }
			std::uint32_t get_packets_lost() const { die_if(!m_peer); return m_peer->packetsLost; }
			std::uint32_t get_mean_round_trip_time_ms() const { die_if(!m_peer); return m_peer->roundTripTime; }

			bool is_valid() const { return m_peer != nullptr; }
			ENetPeer* get_enet_peer() const { return m_peer; }

			friend bool operator==(peer const& a, peer const& b) { return a.m_peer == b.m_peer; }
			friend bool operator!=(peer const& a, peer const& b) { return a.m_peer != b.m_peer; }

		private:

			ENetPeer* m_peer;
		};

	} // enet

} // bump
