#pragma once

#include "bump_result.hpp"

#include <bit>
#include <iostream>
#include <span>

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
		void write_8(std::ostream& os, T value, std::endian endian)
		{
			if constexpr (std::is_same_v<T, bool>)
				return detail::write<8>(os, std::uint8_t{ value }, endian);
			else
				return detail::write<8>(os, value, endian);
		}

		template<class T> void write_8_be(std::ostream& os, T value) { return write_8(os, value, std::endian::big); }
		template<class T> void write_8_le(std::ostream& os, T value) { return write_8(os, value, std::endian::little); }
		template<class T> void write_8_ne(std::ostream& os, T value) { return write_8(os, value, std::endian::native); }

		template<class T>
		void write_16(std::ostream& os, T value, std::endian endian)
		{
			return detail::write<16>(os, value, endian);
		}

		template<class T> void write_16_be(std::ostream& os, T value) { return write_16(os, value, std::endian::big); }
		template<class T> void write_16_le(std::ostream& os, T value) { return write_16(os, value, std::endian::little); }
		template<class T> void write_16_ne(std::ostream& os, T value) { return write_16(os, value, std::endian::native); }

		template<class T>
		void write_32(std::ostream& os, T value, std::endian endian)
		{
			if constexpr (std::is_same_v<T, float>)
				return detail::write<32>(os, std::bit_cast<std::uint32_t>(value), endian);
			else
				return detail::write<32>(os, value, endian);
		}

		template<class T> void write_32_be(std::ostream& os, T value) { return write_32(os, value, std::endian::big); }
		template<class T> void write_32_le(std::ostream& os, T value) { return write_32(os, value, std::endian::little); }
		template<class T> void write_32_ne(std::ostream& os, T value) { return write_32(os, value, std::endian::native); }

		template<class T>
		void write_64(std::ostream& os, T value, std::endian endian)
		{
			if constexpr (std::is_same_v<T, double>)
				return detail::write<64>(os, std::bit_cast<std::uint64_t>(value), endian);
			else
				return detail::write<64>(os, value, endian);
		}

		template<class T> void write_64_be(std::ostream& os, T value) { return write_64(os, value, std::endian::big); }
		template<class T> void write_64_le(std::ostream& os, T value) { return write_64(os, value, std::endian::little); }
		template<class T> void write_64_ne(std::ostream& os, T value) { return write_64(os, value, std::endian::native); }

		template<class T>
		T read_8(std::istream& is, std::endian endian)
		{
			if constexpr (std::is_same_v<T, bool>)
				return detail::read<8, std::uint8_t>(is, endian);
			else
				return detail::read<8, T>(is, endian);
		}

		template<class T> T read_8_be(std::istream& is) { return read_8<T>(is, std::endian::big); }
		template<class T> T read_8_le(std::istream& is) { return read_8<T>(is, std::endian::little); }
		template<class T> T read_8_ne(std::istream& is) { return read_8<T>(is, std::endian::native); }

		template<class T>
		T read_16(std::istream& is, std::endian endian)
		{
			return detail::read<16, T>(is, endian);
		}

		template<class T> T read_16_be(std::istream& is) { return read_16<T>(is, std::endian::big); }
		template<class T> T read_16_le(std::istream& is) { return read_16<T>(is, std::endian::little); }
		template<class T> T read_16_ne(std::istream& is) { return read_16<T>(is, std::endian::native); }

		template<class T>
		T read_32(std::istream& is, std::endian endian)
		{
			if constexpr (std::is_same_v<T, float>)
				return std::bit_cast<float>(detail::read<32, std::uint32_t>(is, endian));
			else
				return detail::read<32, T>(is, endian);
		}

		template<class T> T read_32_be(std::istream& is) { return read_32<T>(is, std::endian::big); }
		template<class T> T read_32_le(std::istream& is) { return read_32<T>(is, std::endian::little); }
		template<class T> T read_32_ne(std::istream& is) { return read_32<T>(is, std::endian::native); }

		template<class T>
		T read_64(std::istream& is, std::endian endian)
		{
			if constexpr (std::is_same_v<T, double>)
				return std::bit_cast<double>(detail::read<64, std::uint64_t>(is, endian));
			else
				return detail::read<64, T>(is, endian);
		}

		template<class T> T read_64_be(std::istream& is) { return read_64<T>(is, std::endian::big); }
		template<class T> T read_64_le(std::istream& is) { return read_64<T>(is, std::endian::little); }
		template<class T> T read_64_ne(std::istream& is) { return read_64<T>(is, std::endian::native); }

		template<class T> struct write_impl;
		template<class T> void write_struct(std::ostream& os, T const& value, std::endian endian) { return write_impl<T>::write(os, value, endian); }

		template<class T> struct read_impl;
		template<class T> T read_struct(std::istream& is, std::endian endian) { return read_impl<T>::read(is, endian); }

	} // io

} // bump
