#pragma once

#include "bump_die.hpp"

#include <enet/enet.h>

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace bump
{

	namespace enet
	{

		class packet
		{
		public:

			packet(): m_packet(nullptr) { }
			explicit packet(ENetPacket* packet): m_packet(packet) { }
			explicit packet(std::string_view data, int flags = 0);
			explicit packet(std::uint8_t const* data, std::size_t data_size, int flags = 0);

			packet(packet const&) = delete;
			packet& operator=(packet const&) = delete;
			packet(packet&& other);
			packet& operator=(packet&& other);

			~packet();

			std::uint32_t get_flags() const { die_if(!m_packet); return m_packet->flags; }
			void set_flags(std::uint32_t flags) { die_if(!m_packet); m_packet->flags = flags; }

			std::uint8_t* data() { die_if(!m_packet); return m_packet->data; }
			std::uint8_t const* data() const { die_if(!m_packet); return m_packet->data; }

			std::size_t size() const { die_if(!m_packet); return m_packet->dataLength; }
			void resize(std::size_t data_size) { die_if(!m_packet); enet_packet_resize(m_packet, data_size); }

			bool is_valid() const { return m_packet != nullptr; }

			ENetPacket* get_enet_packet() const { return m_packet; }
			ENetPacket* release() { auto packet = m_packet; m_packet = nullptr; return packet; }

		private:

			ENetPacket* m_packet;
		};

	} // enet

} // bump
