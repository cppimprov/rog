#pragma once

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

			peer(): m_peer(nullptr) { };
			peer(ENetPeer* peer);
			
			peer(peer const&) = default;
			peer& operator=(peer const&) = default;
			peer(peer&&) = default;
			peer& operator=(peer&&) = default;

			address get_address() const;

			std::uint8_t get_channel_count() const;

			void send(std::uint8_t channel_id, packet packet);
			std::optional<packet> receive(std::uint8_t& channel_id);

			void set_ping_interval(std::uint32_t interval_ms);
			void ping();

			void set_timeout(std::uint32_t limit_ms, std::uint32_t minimum_ms, std::uint32_t maximum_ms);

			void disconnect(std::uint32_t data);
			void disconnect_now(std::uint32_t data);
			void disconnect_later(std::uint32_t data);
			void reset();

			std::uint32_t get_bandwidth_incoming() const;
			std::uint32_t get_bandwidth_outgoing() const;
			std::uint32_t get_total_data_sent() const;
			std::uint32_t get_total_data_received() const;
			std::uint32_t get_packets_sent() const;
			std::uint32_t get_packets_lost() const;
			std::uint32_t get_mean_round_trip_time_ms() const;

			ENetPeer* get_enet_peer() const;

		private:

			ENetPeer* m_peer;
		};

	} // enet

} // bump
