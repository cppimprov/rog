#include "bump_net_ip_enums.hpp"

#include "bump_die.hpp"
#include "bump_net_platform.hpp"

namespace bump
{
	
	namespace net
	{
		
		namespace platform
		{

			int get_ai_family(ip_address_family version)
			{
				switch (version)
				{
				case ip_address_family::V4: return AF_INET;
				case ip_address_family::V6: return AF_INET6;
				case ip_address_family::UNSPECIFIED: return AF_UNSPEC;
				}

				die();
			}

			int get_ai_socktype(ip_protocol protocol)
			{
				switch (protocol)
				{
				case ip_protocol::UDP: return SOCK_DGRAM;
				case ip_protocol::TCP: return SOCK_STREAM;
				}

				die();
			}

			int get_ai_protocol(ip_protocol protocol)
			{
				switch (protocol)
				{
				case ip_protocol::UDP: return IPPROTO_UDP;
				case ip_protocol::TCP: return IPPROTO_TCP;
				}

				die();
			}

		} // platform

	} // net

} // bump
