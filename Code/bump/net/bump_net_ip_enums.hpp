#pragma once

namespace bump
{
	
	namespace net
	{
		
		enum class ip_address_family { V4, V6, UNSPECIFIED };
		enum class ip_protocol { UDP, TCP };

		namespace platform
		{

			int get_ai_family(ip_address_family version);
			int get_ai_socktype(ip_protocol protocol);
			int get_ai_protocol(ip_protocol protocol);

		} // platform

	} // net

} // bump
