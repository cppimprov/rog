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

		// for ease of use, specializations are provided for all of the C++ fundamental types.
		// these assume certain sizes for each that are not necessarily the case on all platforms.

		// when used on platforms where the size does not match, the static_asserts
		// in detail::read and detail::write will be triggered.

		// on these platforms you must convert to and from the relevant fixed-size types
		// or (better) only use fixed size types for values that are to be serialized.

		// the second template parameter (U) in read_impl and write_impl below is used to 
		// prevent full specialization of the template. this means that the static_asserts
		// are not triggered on platforms with different sizes unless the relevant read / write
		// function is actually called.

		template<class T, class U = void>
		struct read_impl;

		template<class U> struct read_impl<bool, U> { static bool read(std::istream& is) { return static_cast<bool>(detail::read<8, std::uint8_t>(is)); } };
		template<class U> struct read_impl<char, U> { static char read(std::istream& is) { return detail::read<8, char>(is); } };
		template<class U> struct read_impl<signed char, U> { static signed char read(std::istream& is) { return detail::read<8, signed char>(is); } };
		template<class U> struct read_impl<unsigned char, U> { static unsigned char read(std::istream& is) { return detail::read<8, unsigned char>(is); } };
		template<class U> struct read_impl<short, U> { static short read(std::istream& is) { return detail::read<16, short>(is); } };
		template<class U> struct read_impl<unsigned short, U> { static unsigned short read(std::istream& is) { return detail::read<16, unsigned short>(is); } };
		template<class U> struct read_impl<int, U> { static int read(std::istream& is) { return detail::read<32, int>(is); } };
		template<class U> struct read_impl<unsigned int, U> { static unsigned int read(std::istream& is) { return detail::read<32, unsigned int>(is); } };
		template<class U> struct read_impl<long, U> { static long read(std::istream& is) { return detail::read<32, long>(is); } };
		template<class U> struct read_impl<unsigned long, U> { static unsigned long read(std::istream& is) { return detail::read<32, unsigned long>(is); } };
		template<class U> struct read_impl<long long, U> { static long long read(std::istream& is) { return detail::read<64, long long>(is); } };
		template<class U> struct read_impl<unsigned long long, U> { static unsigned long long read(std::istream& is) { return detail::read<64, unsigned long long>(is); } };
		template<class U> struct read_impl<float, U> { static float read(std::istream& is) { return std::bit_cast<float>(detail::read<32, std::uint32_t>(is)); } };
		template<class U> struct read_impl<double, U> { static double read(std::istream& is) { return std::bit_cast<double>(detail::read<64, std::uint64_t>(is)); } };
		template<class U> struct read_impl<char8_t, U> { static char8_t read(std::istream& is) { return detail::read<8, char8_t>(is); } };
		template<class U> struct read_impl<char16_t, U> { static char16_t read(std::istream& is) { return detail::read<16, char16_t>(is); } };
		template<class U> struct read_impl<char32_t, U> { static char32_t read(std::istream& is) { return detail::read<32, char32_t>(is); } };
		template<class U> struct read_impl<wchar_t, U> { static wchar_t read(std::istream& is) { return detail::read<16, wchar_t>(is); } };

		template<class T>
		T read(std::istream& is)
		{
			return read_impl<T>::read(is);
		}

		template<class T, class U = void>
		struct write_impl;

		template<class U> struct write_impl<bool, U> { static void write(std::ostream& os, bool value) { detail::write<8>(os, std::uint8_t{ value }); } };
		template<class U> struct write_impl<char, U> { static void write(std::ostream& os, char value) { detail::write<8>(os, value); } };
		template<class U> struct write_impl<signed char, U> { static void write(std::ostream& os, signed char value) { detail::write<8>(os, value); } };
		template<class U> struct write_impl<unsigned char, U> { static void write(std::ostream& os, unsigned char value) { detail::write<8>(os, value); } };
		template<class U> struct write_impl<short, U> { static void write(std::ostream& os, short value) { detail::write<16>(os, value); } };
		template<class U> struct write_impl<unsigned short, U> { static void write(std::ostream& os, unsigned short value) { detail::write<16>(os, value); } };
		template<class U> struct write_impl<int, U> { static void write(std::ostream& os, int value) { detail::write<32>(os, value); } };
		template<class U> struct write_impl<unsigned int, U> { static void write(std::ostream& os, unsigned int value) { detail::write<32>(os, value); } };
		template<class U> struct write_impl<long, U> { static void write(std::ostream& os, long value) { detail::write<32>(os, value); } };
		template<class U> struct write_impl<unsigned long, U> { static void write(std::ostream& os, unsigned long value) { detail::write<32>(os, value); } };
		template<class U> struct write_impl<long long, U> { static void write(std::ostream& os, long long value) { detail::write<64>(os, value); } };
		template<class U> struct write_impl<unsigned long long, U> { static void write(std::ostream& os, unsigned long long value) { detail::write<64>(os, value); } };
		template<class U> struct write_impl<float, U> { static void write(std::ostream& os, float value) { detail::write<32>(os, std::bit_cast<std::uint32_t>(value)); } };
		template<class U> struct write_impl<double, U> { static void write(std::ostream& os, double value) { detail::write<64>(os, std::bit_cast<std::uint64_t>(value)); } };
		template<class U> struct write_impl<char8_t, U> { static void write(std::ostream& os, char8_t value) { detail::write<8>(os, value); } };
		template<class U> struct write_impl<char16_t, U> { static void write(std::ostream& os, char16_t value) { detail::write<16>(os, value); } };
		template<class U> struct write_impl<char32_t, U> { static void write(std::ostream& os, char32_t value) { detail::write<32>(os, value); } };
		template<class U> struct write_impl<wchar_t, U> { static void write(std::ostream& os, wchar_t value) { detail::write<16>(os, value); } };

		template<class T>
		void write(std::ostream& os, T value)
		{
			return write_impl<T>::write(os, value);
		}

	} // io

} // bump
