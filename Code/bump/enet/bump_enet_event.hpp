#pragma once

#include "bump_enet_packet.hpp"
#include "bump_enet_peer.hpp"

#include <cstdint>

#include <enet/enet.h>

namespace bump
{

	namespace enet
	{

		class event
		{
		public:

			event(): m_event{ ENET_EVENT_TYPE_NONE, nullptr, 0, 0, nullptr } { }
			explicit event(ENetEvent event): m_event(event) { }

			event(event const&) = default;
			event& operator=(event const&) = default;
			event(event&&) = default;
			event& operator=(event&&) = default;

			~event();

			enum class type { none, connect, disconnect, receive };
			type get_type() const { return static_cast<type>(m_event.type); }

			peer get_peer() const { return peer(m_event.peer); }

			std::uint8_t get_channel_id() const { return m_event.channelID; }
			std::uint32_t get_data() const { return m_event.data; }

			bool is_valid() const { return m_event.type != ENET_EVENT_TYPE_NONE; }

			packet take_packet() { auto p = packet(m_event.packet); m_event.packet = nullptr; return p; }

		private:

			ENetEvent m_event;
		};
	
	} // enet

} // bump
