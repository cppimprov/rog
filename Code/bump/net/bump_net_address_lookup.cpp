#include "bump_net_address_lookup.hpp"

#include "bump_bit.hpp"
#include "bump_net_byte_order.hpp"
#include "bump_net_platform.hpp"
#include "bump_static_string.hpp"

#include <bit>
#include <charconv>
#include <memory>

namespace bump
{
	
	namespace net
	{
		
		namespace platform
		{

			using addrinfo_ptr = std::unique_ptr<::addrinfo, void(*)(::addrinfo*)>;
			
			// PORT_MAX is the max length of string needed to represent a port.
			// i.e. length of "65535"
			auto constexpr PORT_MAX = 5;

			static static_string<PORT_MAX> get_port_str(std::uint16_t port)
			{
				auto str = static_string<PORT_MAX>(PORT_MAX, '\0');
				auto const tcr = std::to_chars(str.data(), str.data() + PORT_MAX, port);
				die_if(tcr.ec != std::errc());
				str.resize(tcr.ptr - str.data());
				return str;
			}
			
			static char const* get_protocol_name(ip_protocol protocol)
			{
				switch (protocol)
				{
				case ip_protocol::UDP: return "udp";
				case ip_protocol::TCP: return "tcp";
				}

				die();
			}

			static result<addrinfo_ptr, std::system_error> lookup_address(char const* node, char const* service, ip_address_family address_family, ip_protocol protocol, int flags)
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

			static result<void, std::system_error> lookup_name(char* node, std::size_t max_node_length, char* service, std::size_t max_service_length, ::sockaddr_storage const& address, std::size_t address_length, int flags)
			{
				die_if(!node && !service);

				auto const result = ::getnameinfo((::sockaddr const*)&address, (socklen_t)address_length, node, (std::uint32_t)max_node_length, service, (std::uint32_t)max_service_length, flags);
				
				if (result != 0)
					return make_err(std::system_error(std::error_code(result, std::system_category())));
				
				return make_ok();
			}

			result<std::uint16_t, std::system_error> get_port(std::string const& service_name, ip_protocol protocol)
			{
				auto const serv = ::getservbyname(service_name.c_str(), get_protocol_name(protocol));

				if (!serv)
					return make_err(platform::get_last_error());
				
				return make_ok(to_system_byte_order(static_cast<std::uint16_t>(serv->s_port)));
			}

			result<std::string, std::system_error> get_service_name(std::uint16_t port, ip_protocol protocol)
			{
				auto const serv = ::getservbyport(to_network_byte_order(port), get_protocol_name(protocol));

				if (!serv)
					return make_err(platform::get_last_error());

				return make_ok(std::string(serv->s_name));
			}
			
			result<addrinfo_ptr, std::system_error> get_address_info_any(ip_address_family address_family, ip_protocol protocol, std::uint16_t port)
			{
				// todo: avoid port string conversion?
				auto const port_str = get_port_str(port);
				int flags = AI_PASSIVE | AI_NUMERICSERV;
				return lookup_address(nullptr, port_str.data(), address_family, protocol, flags);
			}

			result<addrinfo_ptr, std::system_error> get_address_info_loopback(ip_address_family address_family, ip_protocol protocol, std::uint16_t port)
			{
				// todo: avoid port string conversion?
				auto const port_str = get_port_str(port);
				int flags = AI_NUMERICSERV;
				return lookup_address(nullptr, port_str.data(), address_family, protocol, flags);
			}

			result<addrinfo_ptr, std::system_error> get_address_info(ip_address_family address_family, ip_protocol protocol, std::string const& node_name, std::uint16_t port, bool lookup_cname)
			{
				// todo: avoid port string conversion?
				auto const port_str = get_port_str(port);
				int flags = AI_NUMERICSERV | (lookup_cname ? AI_CANONNAME : 0);
				return lookup_address(node_name.c_str(), port_str.data(), address_family, protocol, flags);
			}
			
			result<addrinfo_ptr, std::system_error> get_address_info(ip_protocol protocol, ip_address const& address, std::uint16_t port)
			{
				// todo: avoid port and ip string conversions?
				auto const ip_str = address_to_static_string(address);
				auto const port_str = get_port_str(port);
				int flags = AI_NUMERICHOST | AI_NUMERICSERV;
				return lookup_address(ip_str.data(), port_str.data(), address.get_address_family(), protocol, flags);
			}

			result<std::string, std::system_error> get_name_info(endpoint const& endpoint, bool qualify_hostname)
			{
				char node[NI_MAXHOST + 1] = { 0 };
				auto const result = lookup_name(node, NI_MAXHOST, nullptr, 0, endpoint.get_address_storage(), endpoint.get_address_length(),
					NI_NAMEREQD | (!qualify_hostname ? NI_NOFQDN : 0));
				
				if (!result.has_value())
					return make_err(result.error());
				
				return make_ok(std::string(node));
			}

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

		} // platform

		result<endpoint, std::system_error> get_endpoint(ip_protocol protocol, ip_address const& address, std::uint16_t port)
		{
			auto info = platform::get_address_info(protocol, address, port);

			if (!info.has_value())
				return make_err(info.error());
			
			return make_ok(platform::get_endpoints(info.value()).at(0));
		}

		result<address_info, std::system_error> get_address_info_any(ip_address_family address_family, ip_protocol protocol, std::uint16_t port)
		{
			auto info = platform::get_address_info_any(address_family, protocol, port);

			if (!info.has_value())
				return make_err(info.error());

			return make_ok(address_info{ {}, {}, platform::get_endpoints(info.value()) });
		}

		result<address_info, std::system_error> get_address_info_loopback(ip_address_family address_family, ip_protocol protocol, std::uint16_t port)
		{
			auto info = platform::get_address_info_loopback(address_family, protocol, port);

			if (!info.has_value())
				return make_err(info.error());

			return make_ok(address_info{ {}, {}, platform::get_endpoints(info.value()) });
		}
		
		result<address_info, std::system_error> get_address_info(ip_address_family address_family, ip_protocol protocol, std::string const& node_name, std::uint16_t port, bool lookup_cname)
		{
			auto info = platform::get_address_info(address_family, protocol, node_name, port, lookup_cname);

			if (!info.has_value())
				return make_err(info.error());
			
			return make_ok(address_info{ node_name, platform::get_canonical_name(info.value()), platform::get_endpoints(info.value()) });
		}

		result<std::string, std::system_error> get_name_info(endpoint const& endpoint, bool qualify_hostname)
		{
			return platform::get_name_info(endpoint, qualify_hostname);
		}

		result<std::uint16_t, std::system_error> get_port(std::string const& service_name, ip_protocol protocol)
		{
			return platform::get_port(service_name, protocol);
		}

		result<std::string, std::system_error> get_service_name(std::uint16_t port, ip_protocol protocol)
		{
			return platform::get_service_name(port, protocol);
		}

	} // net
	
} // bump
