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
			
			result<void, std::system_error> init_socket_library();
			result<void, std::system_error> shutdown_socket_library();
			
			using addrinfo_ptr = std::unique_ptr<::addrinfo, void(*)(::addrinfo*)>;
			
			result<addrinfo_ptr, std::system_error> get_wildcard_address(ip::address_family address_family, ip::protocol protocol);
			result<addrinfo_ptr, std::system_error> get_wildcard_address(std::string const& service_name, ip::name_type service_name_type, ip::address_family address_family, ip::protocol protocol);
			result<addrinfo_ptr, std::system_error> get_loopback_address(ip::address_family address_family, ip::protocol protocol);
			result<addrinfo_ptr, std::system_error> get_loopback_address(std::string const& service_name, ip::name_type service_name_type, ip::address_family address_family, ip::protocol protocol);
			result<addrinfo_ptr, std::system_error> get_address(std::string const& node_name, ip::name_type node_name_type, ip::address_family address_family, ip::protocol protocol);
			result<addrinfo_ptr, std::system_error> get_address(std::string const& node_name, ip::name_type node_name_type, std::string const& service_name, ip::name_type service_name_type, ip::address_family address_family, ip::protocol protocol);
			result<addrinfo_ptr, std::system_error> get_address_and_canonical_name(std::string const& node_name, ip::name_type node_name_type, ip::address_family address_family, ip::protocol protocol);
			result<addrinfo_ptr, std::system_error> get_address_and_canonical_name(std::string const& node_name, ip::name_type node_name_type, std::string const& service_name, ip::name_type service_name_type, ip::address_family address_family, ip::protocol protocol);

			result<std::string, std::system_error> get_node_name(ip::endpoint const& endpoint, ip::qualify_local_host qualify);
			result<std::string, std::system_error> get_node_name(ip::endpoint const& endpoint);
			result<std::string, std::system_error> get_node_ip(ip::endpoint const& endpoint);
			result<std::string, std::system_error> get_port(ip::endpoint const& endpoint);

		} // platform
		
	} // net
	
} // bump
