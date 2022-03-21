#pragma once

namespace bump
{
	
	namespace net
	{
		
		namespace ip
		{

			enum class address_family { V4, V6, UNSPECIFIED };
			enum class protocol { UDP, TCP };

		} // ip

		namespace platform
		{

			int get_ai_family(ip::address_family version);
			int get_ai_socktype(ip::protocol protocol);
			int get_ai_protocol(ip::protocol protocol);

		} // platform

	} // net

} // bump
