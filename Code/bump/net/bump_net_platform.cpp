#include "bump_net_platform.hpp"

#include "bump_bit.hpp"
#include "bump_die.hpp"
#include "bump_net_platform_includes.hpp"
#include "bump_static_string.hpp"

#include <array>
#include <charconv>
#include <cstring>

namespace bump
{
	
	namespace net
	{

		namespace platform
		{

			namespace
			{

				// PORT_MAX is the max length of string needed to represent a port.
				// i.e. length of "65535"
				auto constexpr PORT_MAX = 5;

				static_string<PORT_MAX> get_port_str(std::uint16_t port)
				{
					auto str = static_string<PORT_MAX>(PORT_MAX, '\0');
					auto const tcr = std::to_chars(str.data(), str.data() + PORT_MAX, port);
					die_if(tcr.ec != std::errc());
					str.resize(tcr.ptr - str.data());
					return str;
				}
				
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

				char const* get_protocol_name(ip::protocol protocol)
				{
					switch (protocol)
					{
					case ip::protocol::UDP: return "udp";
					case ip::protocol::TCP: return "tcp";
					}

					die();
				}

			} // unnamed

#if defined(BUMP_NET_WS2)

			namespace
			{

				std::system_error get_last_error()
				{
					return std::system_error(std::error_code(::WSAGetLastError(), std::system_category()));
				}

			} // unnamed

			result<void, std::system_error> init_socket_library()
			{
				auto data = WSADATA();
				auto const result = ::WSAStartup(MAKEWORD(2, 2), &data);

				if (result != 0)
					return make_err(std::system_error(std::error_code(result, std::system_category())));

				return make_ok();
			}

			result<void, std::system_error> shutdown_socket_library()
			{
				auto const result = ::WSACleanup();

				if (result != 0)
					return make_err(get_last_error());

				return make_ok();
			}

			result<addrinfo_ptr, std::system_error> lookup_address(char const* node, char const* service, ip::address_family address_family, ip::protocol protocol, int flags)
			{
				die_if(!node && !service);

				auto hints = ::addrinfo();
				std::memset(&hints, 0, sizeof(::addrinfo));
				hints.ai_family = get_ai_family(address_family);
				hints.ai_socktype = get_ai_socktype(protocol);
				hints.ai_protocol = get_ai_protocol(protocol);
				hints.ai_flags = flags;

				auto out = (::addrinfo*)nullptr;
				auto const result = ::getaddrinfo(node, service, &hints, &out);

				if (result != 0)
					return make_err(std::system_error(std::error_code(result, std::system_category())));

				die_if(!out);

				return make_ok(addrinfo_ptr(out, &::freeaddrinfo));
			}

			result<void, std::system_error> lookup_name(char* node, std::size_t max_node_length, char* service, std::size_t max_service_length, ::sockaddr_storage const& address, std::size_t address_length, int flags)
			{
				die_if(!node && !service);

				auto const result = ::getnameinfo((::sockaddr const*)&address, (socklen_t)address_length, node, (DWORD)max_node_length, service, (DWORD)max_service_length, flags);
				
				if (result != 0)
					return make_err(std::system_error(std::error_code(result, std::system_category())));
				
				return make_ok();
			}

#else

			result<void, error> init_socket_library()
			{
				return make_ok(); // nothing to do!
			}

			result<void, error> shutdown_socket_library()
			{
				return make_ok(); // nothing to do!
			}

#endif

			auto constexpr IP_MAX4 = INET_ADDRSTRLEN;

			static static_string<IP_MAX4> address_to_static_string(::in_addr const& address)
			{
				auto str = static_string<IP_MAX4>(IP_MAX4, '\0');
				auto const result = ::inet_ntop(AF_INET, &address, str.data(), INET_ADDRSTRLEN);
				die_if(!result);
				str.resize(::strnlen_s(str.data(), str.size()));
				return str;
			}

			auto constexpr IP_MAX6 = INET6_ADDRSTRLEN;

			static static_string<IP_MAX6> address_to_static_string(::in6_addr const& address)
			{
				auto str = static_string<IP_MAX6>(IP_MAX6, '\0');
				auto const result = ::inet_ntop(AF_INET6, &address, str.data(), INET6_ADDRSTRLEN);
				die_if(!result);
				str.resize(::strnlen_s(str.data(), str.size()));
				return str;
			}

			auto constexpr IP_MAX = std::max(IP_MAX4, IP_MAX6);

			static_string<IP_MAX> address_to_static_string(ip::address const& address)
			{
				if (address.get_address_family() == ip::address_family::V4) return address_to_static_string(address.data_v4());
				if (address.get_address_family() == ip::address_family::V6) return address_to_static_string(address.data_v6());
				die(); // invalid address family
			}

			std::string address_to_string(::in_addr const& address)
			{
				return address_to_static_string(address).str();
			}
			
			std::string address_to_string(::in6_addr const& address)
			{
				return address_to_static_string(address).str();
			}

			std::optional<ip::address> string_to_address(std::string const& str)
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

			result<std::uint16_t, std::system_error> get_port(std::string const& service_name, ip::protocol protocol)
			{
				auto const serv = ::getservbyname(service_name.c_str(), get_protocol_name(protocol));

				if (!serv)
					return make_err(get_last_error());
				
				return make_ok(to_system_byte_order(static_cast<std::uint16_t>(serv->s_port)));
			}

			result<std::string, std::system_error> get_service_name(std::uint16_t port, ip::protocol protocol)
			{
				auto const serv = ::getservbyport(to_network_byte_order(port), get_protocol_name(protocol));

				if (!serv)
					return make_err(get_last_error());

				return make_ok(std::string(serv->s_name));
			}
			
			result<addrinfo_ptr, std::system_error> get_address_info_any(ip::address_family address_family, ip::protocol protocol, std::uint16_t port)
			{
				// todo: avoid port string conversion?
				auto const port_str = get_port_str(port);
				int flags = AI_PASSIVE | AI_NUMERICSERV;
				return lookup_address(nullptr, port_str.data(), address_family, protocol, flags);
			}

			result<addrinfo_ptr, std::system_error> get_address_info_loopback(ip::address_family address_family, ip::protocol protocol, std::uint16_t port)
			{
				// todo: avoid port string conversion?
				auto const port_str = get_port_str(port);
				int flags = AI_NUMERICSERV;
				return lookup_address(nullptr, port_str.data(), address_family, protocol, flags);
			}

			result<addrinfo_ptr, std::system_error> get_address_info(ip::address_family address_family, ip::protocol protocol, std::string const& node_name, std::uint16_t port, bool lookup_cname)
			{
				// todo: avoid port string conversion?
				auto const port_str = get_port_str(port);
				int flags = AI_NUMERICSERV | (lookup_cname ? AI_CANONNAME : 0);
				return lookup_address(node_name.c_str(), port_str.data(), address_family, protocol, flags);
			}
			
			result<addrinfo_ptr, std::system_error> get_address_info(ip::protocol protocol, ip::address const& address, std::uint16_t port)
			{
				// todo: avoid port and ip string conversions?
				auto const ip_str = address_to_static_string(address);
				auto const port_str = get_port_str(port);
				int flags = AI_NUMERICHOST | AI_NUMERICSERV;
				return lookup_address(ip_str.data(), port_str.data(), address.get_address_family(), protocol, flags);
			}

			result<std::string, std::system_error> get_name_info(ip::endpoint const& endpoint, bool qualify_hostname)
			{
				char node[NI_MAXHOST + 1] = { 0 };
				auto const result = lookup_name(node, NI_MAXHOST, nullptr, 0, endpoint.get_address_storage(), endpoint.get_address_length(),
					NI_NAMEREQD | (!qualify_hostname ? NI_NOFQDN : 0));
				
				if (!result.has_value())
					return make_err(result.error());
				
				return make_ok(std::string(node));
			}

		} // platform
		
	} // net

} // bump
