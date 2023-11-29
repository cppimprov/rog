#pragma once

#include <enet/enet.h>

#include <cstdint>
#include <cstddef>

namespace bump
{

	namespace enet
	{

		// two different mechanics for handling packets:
		// 1. manually create them, and then pass them to enet host.
		// 2. get them from enet events and manually delete them.

		// either way we want an owning pointer...

		class packet
		{
		public:

			packet();
			explicit packet(ENetPacket* packet);
			packet(std::uint8_t const* data, std::size_t data_size);
			packet(std::uint8_t const* data, std::size_t data_size, std::uint8_t flags);

			packet(packet const&) = delete;
			packet& operator=(packet const&) = delete;
			packet(packet&& other);
			packet& operator=(packet&& other);

			~packet();

			std::uint8_t get_flags() const;
			void set_flags(std::uint8_t flags);

			std::uint8_t* data();
			std::uint8_t const* data() const;

			std::size_t size() const;
			void resize(std::size_t data_size);

			ENetPacket* get_enet_packet() const;
			ENetPacket* release();

		private:

			ENetPacket* m_packet;
		};

	} // enet

} // bump
