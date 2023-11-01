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

		template<class T> struct read_impl;

		template<class T>
		T read(std::istream& is)
		{
			return read_impl<std::decay_t<T>>::read(is);
		}

		template<class T> struct write_impl;

		template<class T>
		void write(std::ostream& os, T value)
		{
			return write_impl<std::decay_t<T>>::write(os, value);
		}

	} // io

} // bump
