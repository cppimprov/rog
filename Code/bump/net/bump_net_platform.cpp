#include "bump_net_platform.hpp"

#include "bump_die.hpp"
#include "bump_net_platform_includes.hpp"

namespace bump
{
	
	namespace net
	{

		namespace platform
		{

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

			namespace
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

			result<addrinfo_ptr, std::system_error> get_wildcard_address(ip::address_family address_family, ip::protocol protocol)
			{
				return lookup_address(nullptr, "0", address_family, protocol, AI_PASSIVE);
			}

			result<addrinfo_ptr, std::system_error> get_wildcard_address(std::string const& service_name, ip::name_type service_name_type, ip::address_family address_family, ip::protocol protocol)
			{
				return lookup_address(nullptr, service_name.c_str(), address_family, protocol, 
					AI_PASSIVE | (service_name_type == ip::name_type::NUMERIC ? AI_NUMERICSERV : 0));
			}

			result<addrinfo_ptr, std::system_error> get_loopback_address(ip::address_family address_family, ip::protocol protocol)
			{
				return lookup_address(nullptr, "0", address_family, protocol, 0);
			}

			result<addrinfo_ptr, std::system_error> get_loopback_address(std::string const& service_name, ip::name_type service_name_type, ip::address_family address_family, ip::protocol protocol)
			{
				return lookup_address(nullptr, service_name.c_str(), address_family, protocol,
					(service_name_type == ip::name_type::NUMERIC ? AI_NUMERICSERV : 0));
			}

			result<addrinfo_ptr, std::system_error> get_address(std::string const& node_name, ip::name_type node_name_type, ip::address_family address_family, ip::protocol protocol)
			{
				return lookup_address(node_name.c_str(), "0", address_family, protocol,
					(node_name_type == ip::name_type::NUMERIC ? AI_NUMERICHOST : 0));
			}

			result<addrinfo_ptr, std::system_error> get_address(std::string const& node_name, ip::name_type node_name_type, std::string const& service_name, ip::name_type service_name_type, ip::address_family address_family, ip::protocol protocol)
			{
				return lookup_address(node_name.c_str(), service_name.c_str(), address_family, protocol,
					(node_name_type == ip::name_type::NUMERIC ? AI_NUMERICHOST : 0) | (service_name_type == ip::name_type::NUMERIC ? AI_NUMERICSERV : 0));
			}

			result<addrinfo_ptr, std::system_error> get_address_and_canonical_name(std::string const& node_name, ip::name_type node_name_type, ip::address_family address_family, ip::protocol protocol)
			{
				return lookup_address(node_name.c_str(), "0", address_family, protocol,
					AI_CANONNAME | (node_name_type == ip::name_type::NUMERIC ? AI_NUMERICHOST : 0));
			}

			result<addrinfo_ptr, std::system_error> get_address_and_canonical_name(std::string const& node_name, ip::name_type node_name_type, std::string const& service_name, ip::name_type service_name_type, ip::address_family address_family, ip::protocol protocol)
			{
				return lookup_address(node_name.c_str(), service_name.c_str(), address_family, protocol,
					AI_CANONNAME | (node_name_type == ip::name_type::NUMERIC ? AI_NUMERICHOST : 0) | (service_name_type == ip::name_type::NUMERIC ? AI_NUMERICSERV : 0));
			}

			result<std::string, std::system_error> get_node_name(ip::endpoint const& endpoint, ip::qualify_local_host qualify)
			{
				char node[NI_MAXHOST + 1] = { 0 };
				auto const result = lookup_name(node, NI_MAXHOST, nullptr, 0, endpoint.get_address(), endpoint.get_address_length(),
					NI_NAMEREQD | (qualify == ip::qualify_local_host::hostname_only ? NI_NOFQDN : 0));
				
				if (!result.has_value())
					return make_err(result.error());
				
				return make_ok(std::string(node));
			}

			result<std::string, std::system_error> get_node_name(ip::endpoint const& endpoint)
			{
				char node[NI_MAXHOST + 1] = { 0 };
				auto const result = lookup_name(node, NI_MAXHOST, nullptr, 0, endpoint.get_address(), endpoint.get_address_length(), NI_NAMEREQD);
				
				if (!result.has_value())
					return make_err(result.error());
				
				return make_ok(std::string(node));
			}

			result<std::string, std::system_error> get_node_ip(ip::endpoint const& endpoint)
			{
				char node[NI_MAXHOST + 1] = { 0 };
				auto const result = lookup_name(node, NI_MAXHOST, nullptr, 0, endpoint.get_address(), endpoint.get_address_length(), NI_NUMERICHOST);
				
				if (!result.has_value())
					return make_err(result.error());
				
				return make_ok(std::string(node));
			}

			result<std::string, std::system_error> get_port(ip::endpoint const& endpoint)
			{
				char service[NI_MAXSERV + 1] = { 0 };
				auto const result = lookup_name(nullptr, 0, service, NI_MAXSERV, endpoint.get_address(), endpoint.get_address_length(), NI_NAMEREQD);
				
				if (!result.has_value())
					return make_err(result.error());
				
				return make_ok(std::string(service));
			}
			
		} // platform
		
	} // net

} // bump
