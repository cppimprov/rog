#include "bump_net_endpoint.hpp"

#include "bump_die.hpp"
#include "bump_net_byte_order.hpp"

namespace bump
{
	
	namespace net
	{
		
		namespace platform
		{

			ip_address_family get_ip_address_family(int family)
			{
				switch (family)
				{
				case AF_INET: return ip_address_family::V4;
				case AF_INET6: return ip_address_family::V6;
				case AF_UNSPEC: return ip_address_family::UNSPECIFIED;
				}

				die();
			}

		} // platform
		
		endpoint::endpoint(::addrinfo const& info):
			m_length(info.ai_addrlen),
			m_address()
		{
			die_if(m_length < sizeof(::sockaddr_storage::ss_family));
			die_if(m_length > sizeof(m_address));
			std::memcpy(&m_address, info.ai_addr, m_length);
		}

		ip_address_family endpoint::get_address_family() const
		{
			return platform::get_ip_address_family(m_address.ss_family);
		}

		ip_address endpoint::get_address() const
		{
			auto const family = get_address_family();
			if (family == ip_address_family::V4) return ip_address(get_address_v4().sin_addr);
			if (family == ip_address_family::V6) return ip_address(get_address_v6().sin6_addr);
			die(); // invalid address family
		}

		std::uint16_t endpoint::get_port() const
		{
			auto const family = get_address_family();
			if (family == ip_address_family::V4) return to_system_byte_order(get_address_v4().sin_port);
			if (family == ip_address_family::V6) return to_system_byte_order(get_address_v6().sin6_port);
			die(); // invalid address family
		}

		bool operator==(endpoint const& a, endpoint const& b)
		{
			if (a.m_length != b.m_length) return false;

			auto const a_af = a.get_address_family();
			auto const b_af = b.get_address_family();

			if (a_af != b_af) return false;

			if (a_af == ip_address_family::V4)
			{
				auto const& a_sai = a.get_address_v4();
				auto const& b_sai = b.get_address_v4();

				return
					(std::memcmp(&a_sai.sin_addr, &b_sai.sin_addr, sizeof(::in_addr)) == 0) && 
					(a_sai.sin_port == b_sai.sin_port);
			}

			if (a_af == ip_address_family::V6)
			{
				auto const& a_sai = a.get_address_v6();
				auto const& b_sai = b.get_address_v6();
				
				return
					(std::memcmp(&a_sai.sin6_addr, &b_sai.sin6_addr, sizeof(::in6_addr)) == 0) && 
					(a_sai.sin6_port == b_sai.sin6_port);
			}

			die();
		}
			
	} // net
	
} // bump
