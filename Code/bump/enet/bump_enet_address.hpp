#pragma once

#include <enet/enet.h>

#include <cstdint>

namespace bump
{

	namespace enet
	{
	
		class address
		{
		public:

			address(): m_address{ 0, 0 } { };
			address(std::uint32_t host_be, std::uint16_t port): m_address{ host_be, port } { };
			address(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d, std::uint16_t port);
			explicit address(ENetAddress address): m_address(address) { }
			explicit address(char const* host_name, std::uint16_t port);

			std::uint32_t get_host() const { return m_address.host; }
			void set_host(std::uint32_t host_be) { m_address.host = host_be; }

			std::uint16_t get_port() const { return m_address.port; }
			void set_port(std::uint16_t port) { m_address.port = port; }

			ENetAddress* get_enet_address() { return &m_address; }
			ENetAddress const* get_enet_address() const { return &m_address; }

		private:

			ENetAddress m_address;
		};
	
	} // enet

} // bump
