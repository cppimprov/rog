#include "bump_enet_address.hpp"

namespace bump
{

	namespace enet
	{
	
		address::address(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d, std::uint16_t port)
		{
			m_address.host = ENET_HOST_TO_NET_32((a << 24) | (b << 16) | (c << 8) | d);
			m_address.port = port;
		}

		address::address(char const* host_name, std::uint16_t port)
		{
			enet_address_set_host(&m_address, host_name);
			m_address.port = port;
		}
	
	} // enet

} // bump
