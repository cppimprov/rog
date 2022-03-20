#include "bump_net_ip.hpp"

#include "bump_bit.hpp"
#include "bump_net_platform.hpp"

#include <bit>

namespace bump
{
	
	namespace net
	{
		
		namespace ip
		{

			namespace
			{

				std::vector<endpoint> get_endpoints(platform::addrinfo_ptr const& info)
				{
					if (!info) return { };

					auto out = std::vector<endpoint>();

					auto ptr = info.get();

					while (ptr)
					{
						out.emplace_back(*ptr);
						ptr = ptr->ai_next;
					}

					return out;
				}

				std::string get_canonical_name(platform::addrinfo_ptr const& info)
				{
					if (!info) return { };
					auto const ptr = info.get();
					if (!ptr->ai_canonname) return { };
					return std::string(ptr->ai_canonname);
				}

				ip::address_family get_ip_address_family(int family)
				{
					switch (family)
					{
					case AF_INET: return ip::address_family::V4;
					case AF_INET6: return ip::address_family::V6;
					case AF_UNSPEC: return ip::address_family::UNSPECIFIED;
					}

					die();
				}

			} // unnamed

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
				data[0] = platform::to_network_byte_order(a);
				data[1] = platform::to_network_byte_order(b);
				data[2] = platform::to_network_byte_order(c);
				data[3] = platform::to_network_byte_order(d);
				data[4] = platform::to_network_byte_order(e);
				data[5] = platform::to_network_byte_order(f);
				data[6] = platform::to_network_byte_order(g);
				data[7] = platform::to_network_byte_order(h);
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

			endpoint::endpoint(::addrinfo const& info):
				m_length(info.ai_addrlen),
				m_address()
			{
				die_if(m_length < sizeof(::sockaddr_storage::ss_family));
				die_if(m_length > sizeof(m_address));
				std::memcpy(&m_address, info.ai_addr, m_length);
			}

			address_family endpoint::get_address_family() const
			{
				return get_ip_address_family(m_address.ss_family);
			}

			address endpoint::get_address() const
			{
				auto const family = get_address_family();
				if (family == address_family::V4) return address(get_address_v4().sin_addr);
				if (family == address_family::V6) return address(get_address_v6().sin6_addr);
				die(); // invalid address family
			}

			std::uint16_t endpoint::get_port() const
			{
				auto const family = get_address_family();
				if (family == address_family::V4) return platform::to_system_byte_order(get_address_v4().sin_port);
				if (family == address_family::V6) return platform::to_system_byte_order(get_address_v6().sin6_port);
				die(); // invalid address family
			}

			bool operator==(endpoint const& a, endpoint const& b)
			{
				if (a.m_length != b.m_length) return false;

				auto const a_af = a.get_address_family();
				auto const b_af = b.get_address_family();

				if (a_af != b_af) return false;

				if (a_af == address_family::V4)
				{
					auto const& a_sai = a.get_address_v4();
					auto const& b_sai = b.get_address_v4();

					return
						(std::memcmp(&a_sai.sin_addr, &b_sai.sin_addr, sizeof(::in_addr)) == 0) && 
						(a_sai.sin_port == b_sai.sin_port);
				}

				if (a_af == address_family::V6)
				{
					auto const& a_sai = a.get_address_v6();
					auto const& b_sai = b.get_address_v6();
					
					return
						(std::memcmp(&a_sai.sin6_addr, &b_sai.sin6_addr, sizeof(::in6_addr)) == 0) && 
						(a_sai.sin6_port == b_sai.sin6_port);
				}

				die();
			}

			result<endpoint, std::system_error> get_endpoint(protocol protocol, address const& address, std::uint16_t port)
			{
				auto info = platform::get_address_info(protocol, address, port);

				if (!info.has_value())
					return make_err(info.error());
				
				return make_ok(get_endpoints(info.value()).at(0));
			}

			result<address_info, std::system_error> get_address_info_any(address_family address_family, protocol protocol, std::uint16_t port)
			{
				auto info = platform::get_address_info_any(address_family, protocol, port);

				if (!info.has_value())
					return make_err(info.error());

				return make_ok(address_info{ {}, {}, get_endpoints(info.value()) });
			}

			result<address_info, std::system_error> get_address_info_loopback(address_family address_family, protocol protocol, std::uint16_t port)
			{
				auto info = platform::get_address_info_loopback(address_family, protocol, port);

				if (!info.has_value())
					return make_err(info.error());

				return make_ok(address_info{ {}, {}, get_endpoints(info.value()) });
			}
			
			result<address_info, std::system_error> get_address_info(address_family address_family, protocol protocol, std::string const& node_name, std::uint16_t port, bool lookup_cname)
			{
				auto info = platform::get_address_info(address_family, protocol, node_name, port, lookup_cname);

				if (!info.has_value())
					return make_err(info.error());
				
				return make_ok(address_info{ node_name, get_canonical_name(info.value()), get_endpoints(info.value()) });
			}

			result<std::string, std::system_error> get_name_info(endpoint const& endpoint, bool qualify_hostname)
			{
				return platform::get_name_info(endpoint, qualify_hostname);
			}

			result<std::uint16_t, std::system_error> get_port(std::string const& service_name, protocol protocol)
			{
				return platform::get_port(service_name, protocol);
			}

			result<std::string, std::system_error> get_service_name(std::uint16_t port, protocol protocol)
			{
				return platform::get_service_name(port, protocol);
			}

		} // ip
		
	} // net
	
} // bump
