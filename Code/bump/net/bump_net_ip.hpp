#pragma once

#include "bump_net_platform_includes.hpp"
#include "bump_result.hpp"

#include <cstddef>
#include <string>
#include <system_error>
#include <vector>

namespace bump
{
	
	namespace net
	{
		
		namespace ip
		{
			
			enum class address_family { V4, V6, UNSPECIFIED };
			enum class protocol { UDP, TCP };
			enum class name_type { ANY, NUMERIC };
			enum class qualify_local_host { full, hostname_only };

			class endpoint
			{
			public:

				explicit endpoint(::addrinfo const& info);

				endpoint(endpoint const&) = default;
				endpoint& operator=(endpoint const&) = default;
				endpoint(endpoint&&) = default;
				endpoint& operator=(endpoint&&) = default;

				address_family get_address_family() const;

				std::size_t get_address_length() const { return m_length; }
				::sockaddr_storage const& get_address() const { return m_address; }
				
			private:
				
				std::size_t m_length;
				::sockaddr_storage m_address;
			};
			
			result<std::vector<endpoint>, std::system_error> get_wildcard_address(address_family address_family, protocol protocol);
			result<std::vector<endpoint>, std::system_error> get_wildcard_address(std::string const& service_name, name_type service_name_type, address_family address_family, protocol protocol);
			result<std::vector<endpoint>, std::system_error> get_loopback_address(address_family address_family, protocol protocol);
			result<std::vector<endpoint>, std::system_error> get_loopback_address(std::string const& service_name, name_type service_name_type, address_family address_family, protocol protocol);
			result<std::vector<endpoint>, std::system_error> get_address(std::string const& node_name, name_type node_name_type, address_family address_family, protocol protocol);
			result<std::vector<endpoint>, std::system_error> get_address(std::string const& node_name, name_type node_name_type, std::string const& service_name, name_type service_name_type, address_family address_family, protocol protocol);
			result<std::tuple<std::vector<endpoint>, std::string>, std::system_error> get_address_and_canonical_name(std::string const& node_name, name_type node_name_type, address_family address_family, protocol protocol);
			result<std::tuple<std::vector<endpoint>, std::string>, std::system_error> get_address_and_canonical_name(std::string const& node_name, name_type node_name_type, std::string const& service_name, name_type service_name_type, address_family address_family, protocol protocol);

			result<std::string, std::system_error> get_node_name(endpoint const& endpoint, qualify_local_host qualify);
			result<std::string, std::system_error> get_node_name(endpoint const& endpoint);
			result<std::string, std::system_error> get_node_ip(endpoint const& endpoint);
			result<std::string, std::system_error> get_port(endpoint const& endpoint);
			
		} // ip
		
	} // net
	
} // bump
