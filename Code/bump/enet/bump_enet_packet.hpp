#pragma once

#include <enet/enet.h>

#include <cstdint>
#include <cstddef>

namespace bump
{

	namespace enet
	{

		// todo: non-owning version?
		// events own packets.
		// but we might create a packet that owns itself...

		class packet
		{
		public:

			packet();
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
