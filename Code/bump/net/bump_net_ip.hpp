#pragma once

#include "bump_net_platform_includes.hpp"
#include "bump_result.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <system_error>
#include <variant>
#include <vector>

namespace bump
{
	
	namespace net
	{
		
		namespace ip
		{

			enum class address_family { V4, V6, UNSPECIFIED };
			enum class protocol { UDP, TCP };

			class address
			{
			public:
			
				// todo: implement me!
				//address(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d);
				//address(std::uint16_t a, std::uint16_t b, std::uint16_t c, std::uint16_t d, std::uint16_t e, std::uint16_t f, std::uint16_t g, std::uint16_t h);

				explicit address(::in_addr const& data): m_data(data) { }
				explicit address(::in6_addr const& data): m_data(data) { }

				// todo: initializer list constructor? iterator constructor?

				address_family get_address_family() const { return (m_data.index() == 0) ? address_family::V4 : address_family::V6; }
				::in_addr data_v4() const { return std::get<0>(m_data); }
				::in6_addr data_v6() const { return std::get<1>(m_data); }

			private:

				std::variant<::in_addr, ::in6_addr> m_data;
			};

			// todo: parse address from string!
			result<std::string, std::system_error> to_string(address const& a);

			class endpoint
			{
			public:

				explicit endpoint(::addrinfo const& info);

				endpoint(endpoint const&) = default;
				endpoint& operator=(endpoint const&) = default;
				endpoint(endpoint&&) = default;
				endpoint& operator=(endpoint&&) = default;

				// todo: return a pointer instead, call this function data()
				std::size_t get_address_length() const { return m_length; }
				::sockaddr_storage const& get_address_storage() const { return m_address; }

				address_family get_address_family() const;
				address get_address() const;
				std::uint16_t get_port() const;

			private:
				
				std::size_t m_length;
				::sockaddr_storage m_address;
			};

			struct address_info
			{
				std::string m_node_name; // string used for lookup
				std::string m_canonical_name; // empty unless specifically requested (may be empty even then)
				std::vector<endpoint> m_endpoints;
			};

			result<std::vector<endpoint>, std::system_error> get_address_info_any(address_family address_family, protocol protocol, std::uint16_t port = 0);
			result<std::vector<endpoint>, std::system_error> get_address_info_loopback(address_family address_family, protocol protocol, std::uint16_t port = 0);
			result<std::vector<endpoint>, std::system_error> get_address_info(address_family address_family, protocol protocol, std::string const& node_name, std::uint16_t port = 0, bool lookup_cname = false);
			// todo: return address_info instead (set node name and cname where necessary)
			// todo: version with numeric ip address

			result<std::string, std::system_error> get_name_info(endpoint const& endpoint, bool qualify_hostname = true);
			// todo: version with numeric ip, af, proto ???

			// todo: service name to port conversion (and vice versa):
			// std::uint16_t get_port(std::string const& service_name, ip::protocol);
			// std::string get_service_name(std::uint16_t, ip::protocol); // getservbyname

		} // ip
		
	} // net
	
} // bump
