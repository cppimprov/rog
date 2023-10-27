#pragma once

#include <bit>
#include <iostream>

namespace bump
{

	namespace io
	{

		namespace detail
		{

			template<class U>
			void write_unsigned(std::ostream& os, U value, std::endian e)
			{
				static_assert(std::is_unsigned_v<U>, "type U is not an unsigned type");

				if (e != std::endian::native)
					value = std::byteswap(value);
				
				os.write(reinterpret_cast<char const*>(&value), sizeof(value));
			}

			template<class U>
			U read_unsigned(std::istream& is, std::endian e)
			{
				static_assert(std::is_unsigned_v<U>, "type U is not an unsigned type");

				U bytes;
				is.read(reinterpret_cast<char*>(&bytes), sizeof(bytes));

				if (e != std::endian::native)
					bytes = std::byteswap(bytes);
				
				return bytes;
			}

			template<std::size_t Bits, class T>
			void write(std::ostream& os, T value, std::endian endian)
			{
				static_assert(sizeof(T) * CHAR_BIT == Bits, "type T must be the specified number of bits");
				static_assert(std::is_integral_v<T>, "type T must be an integral type");

				if constexpr (std::is_unsigned_v<T>)
					return detail::write_unsigned(os, value, endian);
				else
					return detail::write_unsigned(os, std::bit_cast<std::make_unsigned_t<T>>(value), endian);
			}

			template<std::size_t Bits, class T>
			T read(std::istream& is, std::endian endian)
			{
				static_assert(sizeof(T) * CHAR_BIT == Bits, "type T must be the specified number of bits");
				static_assert(std::is_integral_v<T>, "type T must be an integral type");

				if constexpr (std::is_unsigned_v<T>)
					return detail::read_unsigned<T>(is, endian);
				else
					return std::bit_cast<T>(detail::read_unsigned<std::make_unsigned_t<T>>(is, endian));
			}

		} // unnamed

		template<class T>
		struct read_impl;

		template<> struct read_impl<bool> { static bool read(std::istream& is, std::endian endian) { return static_cast<bool>(detail::read<8, std::uint8_t>(is, endian)); } };
		template<> struct read_impl<std::int8_t> { static std::int8_t read(std::istream& is, std::endian endian) { return detail::read<8, std::int8_t>(is, endian); } };
		template<> struct read_impl<std::int16_t> { static std::int16_t read(std::istream& is, std::endian endian) { return detail::read<16, std::int16_t>(is, endian); } };
		template<> struct read_impl<std::int32_t> { static std::int32_t read(std::istream& is, std::endian endian) { return detail::read<32, std::int32_t>(is, endian); } };
		template<> struct read_impl<std::int64_t> { static std::int64_t read(std::istream& is, std::endian endian) { return detail::read<64, std::int64_t>(is, endian); } };
		template<> struct read_impl<std::uint8_t> { static std::uint8_t read(std::istream& is, std::endian endian) { return detail::read<8, std::uint8_t>(is, endian); } };
		template<> struct read_impl<std::uint16_t> { static std::uint16_t read(std::istream& is, std::endian endian) { return detail::read<16, std::uint16_t>(is, endian); } };
		template<> struct read_impl<std::uint32_t> { static std::uint32_t read(std::istream& is, std::endian endian) { return detail::read<32, std::uint32_t>(is, endian); } };
		template<> struct read_impl<std::uint64_t> { static std::uint64_t read(std::istream& is, std::endian endian) { return detail::read<64, std::uint64_t>(is, endian); } };
		template<> struct read_impl<float> { static float read(std::istream& is, std::endian endian) { return std::bit_cast<float>(detail::read<32, std::uint32_t>(is, endian)); } };
		template<> struct read_impl<double> { static double read(std::istream& is, std::endian endian) { return std::bit_cast<double>(detail::read<64, std::uint64_t>(is, endian)); } };
		template<> struct read_impl<char8_t> { static char8_t read(std::istream& is, std::endian endian) { return detail::read<8, char8_t>(is, endian); } };
		template<> struct read_impl<char16_t> { static char16_t read(std::istream& is, std::endian endian) { return detail::read<16, char16_t>(is, endian); } };
		template<> struct read_impl<char32_t> { static char32_t read(std::istream& is, std::endian endian) { return detail::read<32, char32_t>(is, endian); } };

		template<class T>
		T read(std::istream& is, std::endian value)
		{
			return read_impl<T>::read(is, value);
		}

		template<class T>
		struct write_impl;

		template<> struct write_impl<bool> { static void write(std::ostream& os, bool value, std::endian endian) { detail::write<8>(os, std::uint8_t{ value }, endian); } };
		template<> struct write_impl<std::int8_t> { static void write(std::ostream& os, std::int8_t value, std::endian endian) { detail::write<8>(os, value, endian); } };
		template<> struct write_impl<std::int16_t> { static void write(std::ostream& os, std::int16_t value, std::endian endian) { detail::write<16>(os, value, endian); } };
		template<> struct write_impl<std::int32_t> { static void write(std::ostream& os, std::int32_t value, std::endian endian) { detail::write<32>(os, value, endian); } };
		template<> struct write_impl<std::int64_t> { static void write(std::ostream& os, std::int64_t value, std::endian endian) { detail::write<64>(os, value, endian); } };
		template<> struct write_impl<std::uint8_t> { static void write(std::ostream& os, std::uint8_t value, std::endian endian) { detail::write<8>(os, value, endian); } };
		template<> struct write_impl<std::uint16_t> { static void write(std::ostream& os, std::uint16_t value, std::endian endian) { detail::write<16>(os, value, endian); } };
		template<> struct write_impl<std::uint32_t> { static void write(std::ostream& os, std::uint32_t value, std::endian endian) { detail::write<32>(os, value, endian); } };
		template<> struct write_impl<std::uint64_t> { static void write(std::ostream& os, std::uint64_t value, std::endian endian) { detail::write<64>(os, value, endian); } };
		template<> struct write_impl<float> { static void write(std::ostream& os, float value, std::endian endian) { detail::write<32>(os, std::bit_cast<std::uint32_t>(value), endian); } };
		template<> struct write_impl<double> { static void write(std::ostream& os, double value, std::endian endian) { detail::write<64>(os, std::bit_cast<std::uint64_t>(value), endian); } };
		template<> struct write_impl<char8_t> { static void write(std::ostream& os, char8_t value, std::endian endian) { detail::write<8>(os, value, endian); } };
		template<> struct write_impl<char16_t> { static void write(std::ostream& os, char16_t value, std::endian endian) { detail::write<16>(os, value, endian); } };
		template<> struct write_impl<char32_t> { static void write(std::ostream& os, char32_t value, std::endian endian) { detail::write<32>(os, value, endian); } };

		template<class T>
		void write(std::ostream& os, T value, std::endian endian)
		{
			return write_impl<T>::write(os, value, endian);
		}

	} // io

} // bump
