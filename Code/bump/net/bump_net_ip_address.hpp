#pragma once

#include "bump_net_ip_enums.hpp"
#include "bump_net_platform.hpp"
#include "bump_static_string.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace bump
{
	
	namespace net
	{
		
		class ip_address
		{
		public:
		
			ip_address(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d);
			ip_address(std::uint16_t a, std::uint16_t b, std::uint16_t c, std::uint16_t d, std::uint16_t e, std::uint16_t f, std::uint16_t g, std::uint16_t h);

			explicit ip_address(::in_addr const& data): m_data(data) { }
			explicit ip_address(::in6_addr const& data): m_data(data) { }

			ip_address(ip_address const&) = default;
			ip_address& operator=(ip_address const&) = default;
			ip_address(ip_address&&) = default;
			ip_address& operator=(ip_address&&) = default;

			ip_address_family get_address_family() const { return (m_data.index() == 0) ? ip_address_family::V4 : ip_address_family::V6; }

			::in_addr& data_v4() { return std::get<0>(m_data); }
			::in_addr const& data_v4() const { return std::get<0>(m_data); }
			::in6_addr& data_v6() { return std::get<1>(m_data); }
			::in6_addr const& data_v6() const { return std::get<1>(m_data); }

			friend bool operator==(ip_address const& a, ip_address const& b);
			friend bool operator!=(ip_address const& a, ip_address const& b) { return !(a == b); }

		private:

			std::variant<::in_addr, ::in6_addr> m_data;
		};

		std::optional<ip_address> try_parse_address(std::string const& str);
		std::string to_string(ip_address const& a);

		namespace platform
		{

			auto constexpr IP_MAX4 = INET_ADDRSTRLEN;
			auto constexpr IP_MAX6 = INET6_ADDRSTRLEN;
			auto constexpr IP_MAX = std::max(IP_MAX4, IP_MAX6);
			static_string<IP_MAX> address_to_static_string(ip_address const& address);
			
		} // platform

	} // net

} // bump
