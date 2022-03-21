#pragma once

#include "bump_net_ip_address.hpp"
#include "bump_net_ip_enums.hpp"
#include "bump_net_platform.hpp"

#include <cstdint>

namespace bump
{
	
	namespace net
	{
		
		namespace ip
		{

			class endpoint
			{
			public:

				explicit endpoint(::addrinfo const& info);

				endpoint(endpoint const&) = default;
				endpoint& operator=(endpoint const&) = default;
				endpoint(endpoint&&) = default;
				endpoint& operator=(endpoint&&) = default;

				address_family get_address_family() const;
				address get_address() const;
				std::uint16_t get_port() const;

				std::size_t get_address_length() const { return m_length; }
				::sockaddr_storage const& get_address_storage() const { return m_address; }
				::sockaddr_in      const& get_address_v4() const { return reinterpret_cast<::sockaddr_in  const&>(m_address); }
				::sockaddr_in6     const& get_address_v6() const { return reinterpret_cast<::sockaddr_in6 const&>(m_address); }

				friend bool operator==(endpoint const& a, endpoint const& b);
				friend bool operator!=(endpoint const& a, endpoint const& b) { return !(a == b); }

			private:
				
				std::size_t m_length;
				::sockaddr_storage m_address;
			};

		} // ip

	} // net

} // bump