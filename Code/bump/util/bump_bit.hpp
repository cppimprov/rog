#pragma once

#include <cstdint>
#include <cstdlib>

namespace bump
{
	
#if defined(_MSC_VER)

				inline std::uint8_t  byte_swap(std::uint8_t  value) { return value; }
				inline std::uint16_t byte_swap(std::uint16_t value) { return _byteswap_ushort(value); }
				inline std::uint32_t byte_swap(std::uint32_t value) { return _byteswap_ulong(value); }
				inline std::uint64_t byte_swap(std::uint64_t value) { return _byteswap_uint64(value); };

#elif defined(__GNUC__)

				inline std::uint8_t  byte_swap(std::uint8_t  value) { return value; }
				inline std::uint16_t byte_swap(std::uint16_t value) { return __builtin_bswap16(value); }
				inline std::uint32_t byte_swap(std::uint32_t value) { return __builtin_bswap32(value); }
				inline std::uint64_t byte_swap(std::uint64_t value) { return __builtin_bswap64(value); };

#else

#error "Unknown compiler! Please define appropriate byte swap functions."

#endif
	
} // bump
