#pragma once

#include "bump_bit.hpp"

#include <bit>
#include <cstdlib>

namespace bump
{
	
	namespace net
	{
		
		inline std::uint8_t  to_network_byte_order(std::uint8_t  value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
		inline std::uint16_t to_network_byte_order(std::uint16_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
		inline std::uint32_t to_network_byte_order(std::uint32_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
		inline std::uint64_t to_network_byte_order(std::uint64_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }

		inline std::uint8_t  to_system_byte_order(std::uint8_t  value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
		inline std::uint16_t to_system_byte_order(std::uint16_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
		inline std::uint32_t to_system_byte_order(std::uint32_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
		inline std::uint64_t to_system_byte_order(std::uint64_t value) { return (std::endian::native == std::endian::big ? value : byte_swap(value)); }
		
	} // net
	
} // bump
