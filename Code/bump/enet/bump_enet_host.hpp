#pragma once

#include "bump_enet_address.hpp"
#include "bump_enet_event.hpp"
#include "bump_enet_peer.hpp"
#include "bump_ptr_handle.hpp"

#include <enet/enet.h>

#include <optional>

namespace bump
{

	namespace enet
	{

		class host : public ptr_handle<ENetHost>
		{
		public:

			host() = default;
			explicit host(ENetHost* host);
			explicit host(address addr, std::size_t peers);
			explicit host(address addr, std::size_t peers, std::uint8_t channels, std::uint32_t bandwidth_in, std::uint32_t bandwidth_out);

			host(host const&) = delete;
			host& operator=(host const&) = delete;
			host(host&&) = default;
			host& operator=(host&&) = default;

			address get_address() const;

			std::size_t get_peer_count() const;
			std::size_t get_connected_peer_count() const;

			// note: this would be UB...
			peer const* peer_begin() const;
			peer* peer_begin();
			peer const* peer_end() const;
			peer* peer_end();

			std::uint32_t get_total_data_sent() const;
			std::uint32_t get_total_data_received() const;
			std::uint32_t get_total_packets_sent() const;
			std::uint32_t get_total_packets_received() const;
			void reset_totals();

			std::uint8_t get_channel_limit() const;
			void set_channel_limit(std::uint8_t limit);

			std::uint32_t get_incoming_bandwidth_limit() const;
			void set_incoming_bandwidth_limit(std::uint32_t limit);

			std::uint32_t get_outgoing_bandwidth_limit() const;
			void set_outgoing_bandwidth_limit(std::uint32_t limit);

			peer connect(address addr, std::size_t channels, std::uint32_t user_data);
			
			std::optional<event> check_events();
			std::optional<event> service(std::uint32_t timeout_ms);
			
			void broadcast(std::uint8_t channel_id, packet packet);
			void flush();

			ENetHost* get_enet_host() const;
		};
	
	} // enet

} // bump
