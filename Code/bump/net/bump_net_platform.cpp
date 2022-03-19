#include "bump_net_platform.hpp"

#include "bump_die.hpp"
#include "bump_net_platform_includes.hpp"

#include <array>
#include <charconv>

namespace bump
{
	
	namespace net
	{

		namespace platform
		{

			namespace
			{

				// PORT_MAX is the max length of string needed to represent a port.
				// i.e. length of "65535" + a null.
				auto constexpr PORT_MAX = 5 + 1;

				std::array<char, PORT_MAX> get_port_str(std::uint16_t port)
				{
					auto str = std::array<char, PORT_MAX>{ 0 };
					auto const tcr = std::to_chars(str.data(), str.data() + PORT_MAX, port);
					die_if(tcr.ec != std::errc());
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
					case ip::protocol::TCP: return SOCK_STREAM;
					case ip::protocol::UDP: return SOCK_DGRAM;
					}

					die();
				}

				int get_ai_protocol(ip::protocol protocol)
				{
					switch (protocol)
					{
					case ip::protocol::TCP: return IPPROTO_TCP;
					case ip::protocol::UDP: return IPPROTO_UDP;
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

			// todo: tidy this!!! remove duplication

			auto constexpr IP_MAX = INET6_ADDRSTRLEN + 1;

			std::array<char, IP_MAX> addr_to_string_static(ip::address const& address)
			{
				auto str = std::array<char, IP_MAX>{ 0 };
				
				if (address.get_address_family() == ip::address_family::V4)
				{
					auto const result = ::inet_ntop(AF_INET, &address.data_v4(), str.data(), INET_ADDRSTRLEN);
					die_if(!result);

					return str;
				}

				if (address.get_address_family() == ip::address_family::V6)
				{
					auto const result = ::inet_ntop(AF_INET6, &address.data_v6(), str.data(), INET6_ADDRSTRLEN);
					die_if(!result);

					return str;
				}

				die();
			}

			std::string addr_to_string(::in_addr const& address)
			{
				char str[INET_ADDRSTRLEN + 1] = { 0 };
				auto const result = ::inet_ntop(AF_INET, &address, str, INET_ADDRSTRLEN);
				die_if(!result);

				return std::string(str);
			}
			
			std::string addr_to_string(::in6_addr const& address)
			{
				char str[INET6_ADDRSTRLEN + 1] = { 0 };
				auto const result = ::inet_ntop(AF_INET6, &address, str, INET6_ADDRSTRLEN);
				die_if(!result);
				
				return std::string(str);
			}

			std::optional<ip::address> string_to_addr(std::string const& str)
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
				auto const ip_str = addr_to_string_static(address);
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
			
			std::optional<ip::address> string_to_address(std::string const& str)
			{
				return string_to_addr(str);
			}

			std::string address_to_string(::in_addr const& address)
			{
				return addr_to_string(address);
			}

			std::string address_to_string(::in6_addr const& address)
			{
				return addr_to_string(address);
			}
			
		} // platform
		
	} // net

} // bump
