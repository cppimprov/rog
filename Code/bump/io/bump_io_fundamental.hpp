#pragma once

#include "bump_io_read_write.hpp"


namespace bump
{

	namespace io
	{

		template<> struct read_impl<bool> { static bool read(std::istream& is) { return static_cast<bool>(detail::read<8, std::uint8_t>(is)); } };
		template<> struct read_impl<char> { static char read(std::istream& is) { return static_cast<char>(detail::read<8, std::uint8_t>(is)); } };
		template<> struct read_impl<std::int8_t> { static signed char read(std::istream& is) { return detail::read<8, std::int8_t>(is); } };
		template<> struct read_impl<std::int16_t> { static short read(std::istream& is) { return detail::read<16, std::int16_t>(is); } };
		template<> struct read_impl<std::int32_t> { static int read(std::istream& is) { return detail::read<32, std::int32_t>(is); } };
		template<> struct read_impl<std::int64_t> { static long long read(std::istream& is) { return detail::read<64, std::int64_t>(is); } };
		template<> struct read_impl<std::uint8_t> { static unsigned char read(std::istream& is) { return detail::read<8, std::uint8_t>(is); } };
		template<> struct read_impl<std::uint16_t> { static unsigned short read(std::istream& is) { return detail::read<16, std::uint16_t>(is); } };
		template<> struct read_impl<std::uint32_t> { static unsigned int read(std::istream& is) { return detail::read<32, std::uint32_t>(is); } };
		template<> struct read_impl<std::uint64_t> { static unsigned long long read(std::istream& is) { return detail::read<64, std::uint64_t>(is); } };
		template<> struct read_impl<float> { static float read(std::istream& is) { return std::bit_cast<float>(detail::read<32, std::uint32_t>(is)); } };
		template<> struct read_impl<double> { static double read(std::istream& is) { return std::bit_cast<double>(detail::read<64, std::uint64_t>(is)); } };
		template<> struct read_impl<char8_t> { static char8_t read(std::istream& is) { return detail::read<8, char8_t>(is); } };
		template<> struct read_impl<char16_t> { static char16_t read(std::istream& is) { return detail::read<16, char16_t>(is); } };
		template<> struct read_impl<char32_t> { static char32_t read(std::istream& is) { return detail::read<32, char32_t>(is); } };

		template<> struct write_impl<bool> { static void write(std::ostream& os, bool value) { detail::write<8>(os, std::uint8_t{ value }); } };
		template<> struct write_impl<char> { static void write(std::ostream& os, char value) { detail::write<8>(os, static_cast<std::uint8_t>(value)); } };
		template<> struct write_impl<std::int8_t> { static void write(std::ostream& os, std::int8_t value) { detail::write<8>(os, value); } };
		template<> struct write_impl<std::int16_t> { static void write(std::ostream& os, std::int16_t value) { detail::write<16>(os, value); } };
		template<> struct write_impl<std::int32_t> { static void write(std::ostream& os, std::int32_t value) { detail::write<32>(os, value); } };
		template<> struct write_impl<std::int64_t> { static void write(std::ostream& os, std::int64_t value) { detail::write<64>(os, value); } };
		template<> struct write_impl<std::uint8_t> { static void write(std::ostream& os, std::uint8_t value) { detail::write<8>(os, value); } };
		template<> struct write_impl<std::uint16_t> { static void write(std::ostream& os, std::uint16_t value) { detail::write<16>(os, value); } };
		template<> struct write_impl<std::uint32_t> { static void write(std::ostream& os, std::uint32_t value) { detail::write<32>(os, value); } };
		template<> struct write_impl<std::uint64_t> { static void write(std::ostream& os, std::uint64_t value) { detail::write<64>(os, value); } };
		template<> struct write_impl<float> { static void write(std::ostream& os, float value) { detail::write<32>(os, std::bit_cast<std::uint32_t>(value)); } };
		template<> struct write_impl<double> { static void write(std::ostream& os, double value) { detail::write<64>(os, std::bit_cast<std::uint64_t>(value)); } };
		template<> struct write_impl<char8_t> { static void write(std::ostream& os, char8_t value) { detail::write<8>(os, value); } };
		template<> struct write_impl<char16_t> { static void write(std::ostream& os, char16_t value) { detail::write<16>(os, value); } };
		template<> struct write_impl<char32_t> { static void write(std::ostream& os, char32_t value) { detail::write<32>(os, value); } };

	} // io

} // bump
