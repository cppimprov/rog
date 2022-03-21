#include "bump_net_ip_enums.hpp"

#include "bump_die.hpp"
#include "bump_net_platform.hpp"

namespace bump
{
	
	namespace net
	{
		
		namespace platform
		{

			int get_ai_family(ip::address_family version)
			{
				switch (version)
				{
				case ip::address_family::V4: return AF_INET;
				case ip::address_family::V6: return AF_INET6;
				case ip::address_family::UNSPECIFIED: return AF_UNSPEC;
				}

				die();
			}

			int get_ai_socktype(ip::protocol protocol)
			{
				switch (protocol)
				{
				case ip::protocol::UDP: return SOCK_DGRAM;
				case ip::protocol::TCP: return SOCK_STREAM;
				}

				die();
			}

			int get_ai_protocol(ip::protocol protocol)
			{
				switch (protocol)
				{
				case ip::protocol::UDP: return IPPROTO_UDP;
				case ip::protocol::TCP: return IPPROTO_TCP;
				}

				die();
			}

		} // platform

	} // net

} // bump
