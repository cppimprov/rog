#pragma once

#include "bump_bit.hpp"
#include "bump_net_ip.hpp"
#include "bump_net_platform_includes.hpp"
#include "bump_result.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <system_error>

namespace bump
{
	
	namespace net
	{
		
		namespace platform
		{
			inline std::uint8_t  to_network_byte_order(std::uint8_t  value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
			inline std::uint16_t to_network_byte_order(std::uint16_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
			inline std::uint32_t to_network_byte_order(std::uint32_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
			inline std::uint64_t to_network_byte_order(std::uint64_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }

			inline std::uint8_t  to_system_byte_order(std::uint8_t  value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
			inline std::uint16_t to_system_byte_order(std::uint16_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
			inline std::uint32_t to_system_byte_order(std::uint32_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
			inline std::uint64_t to_system_byte_order(std::uint64_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }

			result<void, std::system_error> init_socket_library();
			result<void, std::system_error> shutdown_socket_library();
			
			using addrinfo_ptr = std::unique_ptr<::addrinfo, void(*)(::addrinfo*)>;
			
			std::optional<ip::address> string_to_address(std::string const& str);
			std::string address_to_string(::in_addr const& address);
			std::string address_to_string(::in6_addr const& address);
			
			result<addrinfo_ptr, std::system_error> get_address_info_any(ip::address_family address_family, ip::protocol protocol, std::uint16_t port);
			result<addrinfo_ptr, std::system_error> get_address_info_loopback(ip::address_family address_family, ip::protocol protocol, std::uint16_t port);
			result<addrinfo_ptr, std::system_error> get_address_info(ip::address_family address_family, ip::protocol protocol, std::string const& node_name, std::uint16_t port, bool lookup_cname);
			result<addrinfo_ptr, std::system_error> get_address_info(ip::protocol protocol, ip::address const& address, std::uint16_t port);

			result<std::string, std::system_error> get_name_info(ip::endpoint const& endpoint, bool qualify_hostname);

			result<std::uint16_t, std::system_error> get_port(std::string const& service_name, ip::protocol protocol);
			result<std::string, std::system_error> get_service_name(std::uint16_t port, ip::protocol protocol);

		} // platform
		
	} // net
	
} // bump
