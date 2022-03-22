#pragma once

#include "bump_net_endpoint.hpp"
#include "bump_net_ip_address.hpp"
#include "bump_net_ip_enums.hpp"
#include "bump_result.hpp"

#include <string>
#include <system_error>
#include <vector>

namespace bump
{
	
	namespace net
	{
		
		struct address_info
		{
			std::string m_node_name; // string used for lookup
			std::string m_canonical_name; // empty unless specifically requested (may be empty even then)
			std::vector<endpoint> m_endpoints;
		};

		result<endpoint, std::system_error> get_endpoint(ip_protocol protocol, ip_address const& address, std::uint16_t port = 0);

		result<address_info, std::system_error> get_address_info_any(ip_address_family address_family, ip_protocol protocol, std::uint16_t port = 0);
		result<address_info, std::system_error> get_address_info_loopback(ip_address_family address_family, ip_protocol protocol, std::uint16_t port = 0);
		result<address_info, std::system_error> get_address_info(ip_address_family address_family, ip_protocol protocol, std::string const& node_name, std::uint16_t port = 0, bool lookup_cname = false);

		result<std::string, std::system_error> get_name_info(endpoint const& endpoint, bool qualify_hostname = true);

		result<std::uint16_t, std::system_error> get_port(std::string const& service_name, ip_protocol protocol);
		result<std::string, std::system_error> get_service_name(std::uint16_t port, ip_protocol protocol);

	} // net

} // bump
