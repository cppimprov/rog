#pragma once

#include "bump_net_ip.hpp"
#include "bump_net_platform_includes.hpp"
#include "bump_result.hpp"

#include <functional>
#include <memory>
#include <system_error>

namespace bump
{
	
	namespace net
	{
		
		namespace platform
		{
			// todo: put implementation functions in the .cpp file in an unnamed namespace

			result<void, std::system_error> init_socket_library();
			result<void, std::system_error> shutdown_socket_library();
			
			using addrinfo_ptr = std::unique_ptr<::addrinfo, void(*)(::addrinfo*)>;
			
			result<addrinfo_ptr, std::system_error> get_address_info_any(ip::address_family address_family, ip::protocol protocol, std::uint16_t port);
			result<addrinfo_ptr, std::system_error> get_address_info_loopback(ip::address_family address_family, ip::protocol protocol, std::uint16_t port);
			result<addrinfo_ptr, std::system_error> get_address_info(ip::address_family address_family, ip::protocol protocol, std::string const& node_name, std::uint16_t port, bool lookup_cname);

			result<std::string, std::system_error> get_name_info(ip::endpoint const& endpoint, bool qualify_hostname);

			result<std::string, std::system_error> address_to_string(::in_addr const& address);
			result<std::string, std::system_error> address_to_string(::in6_addr const& address);

		} // platform
		
	} // net
	
} // bump
