#pragma once

#include <bit>
#include <iostream>

namespace bump
{

	namespace io
	{

		namespace detail
		{

			extern int endian_index;

		} // detail

		inline void set_endian(std::ios_base& s, std::endian endian)
		{
			s.iword(detail::endian_index) = (endian == std::endian::little ? 1L : 0L);
		}

		inline std::endian get_endian(std::ios_base& s)
		{
			return (s.iword(detail::endian_index) == 1L ? std::endian::little : std::endian::big);
		}

		namespace detail
		{

			template<class U>
			void write_unsigned(std::ostream& os, U value)
			{
				static_assert(std::is_unsigned_v<U>, "type U is not an unsigned type");
				static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little, "mixed endian systems are not supported");

				if (get_endian(os) != std::endian::native)
					value = std::byteswap(value);
				
				os.write(reinterpret_cast<char const*>(&value), sizeof(value));
			}

			template<class U>
			U read_unsigned(std::istream& is)
			{
				static_assert(std::is_unsigned_v<U>, "type U is not an unsigned type");
				static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little, "mixed endian systems are not supported");

				U bytes;
				is.read(reinterpret_cast<char*>(&bytes), sizeof(bytes));

				if (get_endian(is) != std::endian::native)
					bytes = std::byteswap(bytes);
				
				return bytes;
			}

			template<std::size_t Bits, class T>
			void write(std::ostream& os, T value)
			{
				static_assert(sizeof(T) * CHAR_BIT == Bits, "type T must be the specified number of bits");
				static_assert(std::is_integral_v<T>, "type T must be an integral type");

				if constexpr (std::is_unsigned_v<T>)
					return detail::write_unsigned(os, value);
				else
					return detail::write_unsigned(os, std::bit_cast<std::make_unsigned_t<T>>(value));
			}

			template<std::size_t Bits, class T>
			T read(std::istream& is)
			{
				static_assert(sizeof(T) * CHAR_BIT == Bits, "type T must be the specified number of bits");
				static_assert(std::is_integral_v<T>, "type T must be an integral type");

				if constexpr (std::is_unsigned_v<T>)
					return detail::read_unsigned<T>(is);
				else
					return std::bit_cast<T>(detail::read_unsigned<std::make_unsigned_t<T>>(is));
			}

		} // detail

		template<class T>
		struct read_impl;

		template<> struct read_impl<bool> { static bool read(std::istream& is) { return static_cast<bool>(detail::read<8, std::uint8_t>(is)); } };
		template<> struct read_impl<char> { static char read(std::istream& is) { return static_cast<char>(detail::read<8, std::uint8_t>(is)); } };
		template<> struct read_impl<std::int8_t> { static signed char read(std::istream& is) { return detail::read<8, std::int8_t>(is); } };
		template<> struct read_impl<std::uint8_t> { static unsigned char read(std::istream& is) { return detail::read<8, std::uint8_t>(is); } };
		template<> struct read_impl<std::int16_t> { static short read(std::istream& is) { return detail::read<16, std::int16_t>(is); } };
		template<> struct read_impl<std::uint16_t> { static unsigned short read(std::istream& is) { return detail::read<16, std::uint16_t>(is); } };
		template<> struct read_impl<std::int32_t> { static int read(std::istream& is) { return detail::read<32, std::int32_t>(is); } };
		template<> struct read_impl<std::uint32_t> { static unsigned int read(std::istream& is) { return detail::read<32, std::uint32_t>(is); } };
		template<> struct read_impl<std::int64_t> { static long long read(std::istream& is) { return detail::read<64, std::int64_t>(is); } };
		template<> struct read_impl<std::uint64_t> { static unsigned long long read(std::istream& is) { return detail::read<64, std::uint64_t>(is); } };
		template<> struct read_impl<float> { static float read(std::istream& is) { return std::bit_cast<float>(detail::read<32, std::uint32_t>(is)); } };
		template<> struct read_impl<double> { static double read(std::istream& is) { return std::bit_cast<double>(detail::read<64, std::uint64_t>(is)); } };
		template<> struct read_impl<char8_t> { static char8_t read(std::istream& is) { return detail::read<8, char8_t>(is); } };
		template<> struct read_impl<char16_t> { static char16_t read(std::istream& is) { return detail::read<16, char16_t>(is); } };
		template<> struct read_impl<char32_t> { static char32_t read(std::istream& is) { return detail::read<32, char32_t>(is); } };

		template<class T>
		T read(std::istream& is)
		{
			return read_impl<T>::read(is);
		}

		template<class T, class U = void>
		struct write_impl;

		template<> struct write_impl<bool> { static void write(std::ostream& os, bool value) { detail::write<8>(os, std::uint8_t{ value }); } };
		template<> struct write_impl<char> { static void write(std::ostream& os, char value) { detail::write<8>(os, static_cast<std::uint8_t>(value)); } };
		template<> struct write_impl<std::int8_t> { static void write(std::ostream& os, std::int8_t value) { detail::write<8>(os, value); } };
		template<> struct write_impl<std::uint8_t> { static void write(std::ostream& os, std::uint8_t value) { detail::write<8>(os, value); } };
		template<> struct write_impl<std::int16_t> { static void write(std::ostream& os, std::int16_t value) { detail::write<16>(os, value); } };
		template<> struct write_impl<std::uint16_t> { static void write(std::ostream& os, std::uint16_t value) { detail::write<16>(os, value); } };
		template<> struct write_impl<std::int32_t> { static void write(std::ostream& os, std::int32_t value) { detail::write<32>(os, value); } };
		template<> struct write_impl<std::uint32_t> { static void write(std::ostream& os, std::uint32_t value) { detail::write<32>(os, value); } };
		template<> struct write_impl<float> { static void write(std::ostream& os, float value) { detail::write<32>(os, std::bit_cast<std::uint32_t>(value)); } };
		template<> struct write_impl<double> { static void write(std::ostream& os, double value) { detail::write<64>(os, std::bit_cast<std::uint64_t>(value)); } };
		template<> struct write_impl<char8_t> { static void write(std::ostream& os, char8_t value) { detail::write<8>(os, value); } };
		template<> struct write_impl<char16_t> { static void write(std::ostream& os, char16_t value) { detail::write<16>(os, value); } };
		template<> struct write_impl<char32_t> { static void write(std::ostream& os, char32_t value) { detail::write<32>(os, value); } };

		template<class T>
		void write(std::ostream& os, T value)
		{
			return write_impl<T>::write(os, value);
		}

	} // io

} // bump
