#include "bump_net_ip_address.hpp"

#include "bump_die.hpp"
#include "bump_net_byte_order.hpp"
#include "bump_net_platform.hpp"
#include "bump_static_string.hpp"

namespace bump
{
	
	namespace net
	{
		
		namespace platform
		{

#if defined(BUMP_NET_WS2)

			static std::size_t c_strnlen(char const* str, std::size_t max_size)
			{
				return ::strnlen_s(str, max_size);
			}

#else

			static std::size_t c_strnlen(char const* str, std::size_t max_size)
			{
				return ::strnlen(str, max_size);
			}

#endif

			static std::optional<ip::address> string_to_address(std::string const& str)
			{
				// try to parse ipv4 address
				{
					auto addr = ::in_addr();
					auto result = ::inet_pton(AF_INET, str.c_str(), &addr);
					die_if(result == -1);
					
					if (result == 1) return ip::address(addr); // success
				}

				// try to parse ipv6 address
				{
					auto addr = ::in6_addr();
					auto result = ::inet_pton(AF_INET6, str.c_str(), &addr);
					die_if(result == -1);
					
					if (result == 1) return ip::address(addr); // success
				}

				return std::nullopt;
			}

			static static_string<IP_MAX4> address_to_static_string(::in_addr const& address)
			{
				auto str = static_string<IP_MAX4>(IP_MAX4, '\0');
				auto const result = ::inet_ntop(AF_INET, &address, str.data(), INET_ADDRSTRLEN);
				die_if(!result);
				str.resize(c_strnlen(str.data(), str.size()));
				return str;
			}


			static static_string<IP_MAX6> address_to_static_string(::in6_addr const& address)
			{
				auto str = static_string<IP_MAX6>(IP_MAX6, '\0');
				auto const result = ::inet_ntop(AF_INET6, &address, str.data(), INET6_ADDRSTRLEN);
				die_if(!result);
				str.resize(c_strnlen(str.data(), str.size()));
				return str;
			}

			static_string<IP_MAX> address_to_static_string(ip::address const& address)
			{
				if (address.get_address_family() == ip::address_family::V4) return address_to_static_string(address.data_v4());
				if (address.get_address_family() == ip::address_family::V6) return address_to_static_string(address.data_v6());
				die(); // invalid address family
			}

			static std::string address_to_string(::in_addr const& address)
			{
				return address_to_static_string(address).str();
			}
			
			static std::string address_to_string(::in6_addr const& address)
			{
				return address_to_static_string(address).str();
			}

		} // platform

		namespace ip
		{

			address::address(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d):
				m_data(std::in_place_index_t<0>())
			{
				auto const data = reinterpret_cast<std::uint8_t *>(&data_v4());
				data[0] = a; data[1] = b; data[2] = c; data[3] = d;
			}

			address::address(std::uint16_t a, std::uint16_t b, std::uint16_t c, std::uint16_t d, std::uint16_t e, std::uint16_t f, std::uint16_t g, std::uint16_t h):
				m_data(std::in_place_index_t<1>())
			{
				auto const data = reinterpret_cast<std::uint16_t *>(&data_v6());
				data[0] = to_network_byte_order(a);
				data[1] = to_network_byte_order(b);
				data[2] = to_network_byte_order(c);
				data[3] = to_network_byte_order(d);
				data[4] = to_network_byte_order(e);
				data[5] = to_network_byte_order(f);
				data[6] = to_network_byte_order(g);
				data[7] = to_network_byte_order(h);
			}

			bool operator==(address const& a, address const& b)
			{
				auto const a_af = a.get_address_family();
				auto const b_af = b.get_address_family();

				if (a_af != b_af) return false;
				if (a_af == address_family::V4) return (std::memcmp(&a.data_v4(), &b.data_v4(), sizeof(::in_addr)) == 0);
				if (a_af == address_family::V6) return (std::memcmp(&a.data_v6(), &b.data_v6(), sizeof(::in6_addr)) == 0);

				die();
			}

			std::optional<address> try_parse_address(std::string const& str)
			{
				return platform::string_to_address(str);
			}

			std::string to_string(address const& a)
			{
				if (a.get_address_family() == address_family::V4) return platform::address_to_string(a.data_v4());
				if (a.get_address_family() == address_family::V6) return platform::address_to_string(a.data_v6());
				die(); // invalid address family
			}

		} // ip

	} // net

} // bump
