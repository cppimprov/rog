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

			template<class Arg>
			struct stream_manipulator
			{
				stream_manipulator(void(*fn)(std::ios_base&, Arg), Arg arg):
					m_fn(fn),
					m_arg(arg)
				{ }
				
				template<class Elem, class Traits>
				friend std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& is, stream_manipulator const& m)
				{
					m.m_fn(is, m.m_arg);
					return is;
				}

				template<class Elem, class Traits>
				friend std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& os, stream_manipulator const& m)
				{
					m.m_fn(os, m.m_arg);
					return os;
				}

				void(*m_fn)(std::ios_base&, Arg);
				Arg m_arg;
			};
			
			void setendian_impl(std::ios_base& s, std::endian endian);
			std::endian getendian_impl(std::ios_base& s);

		} // detail

		detail::stream_manipulator<std::endian> setendian(std::endian endian);
		std::endian getendian(std::ios_base& s);

		namespace detail
		{

			template<class U>
			void write_unsigned(std::ostream& os, U value)
			{
				static_assert(std::is_unsigned_v<U>, "type U is not an unsigned type");
				static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little, "mixed endian systems are not supported");

				if (getendian(os) != std::endian::native)
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

				if (getendian(is) != std::endian::native)
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

		} // unnamed

		template<class T>
		struct read_impl;

		template<> struct read_impl<bool> { static bool read(std::istream& is) { return static_cast<bool>(detail::read<8, std::uint8_t>(is)); } };
		template<> struct read_impl<std::int8_t> { static std::int8_t read(std::istream& is) { return detail::read<8, std::int8_t>(is); } };
		template<> struct read_impl<std::int16_t> { static std::int16_t read(std::istream& is) { return detail::read<16, std::int16_t>(is); } };
		template<> struct read_impl<std::int32_t> { static std::int32_t read(std::istream& is) { return detail::read<32, std::int32_t>(is); } };
		template<> struct read_impl<std::int64_t> { static std::int64_t read(std::istream& is) { return detail::read<64, std::int64_t>(is); } };
		template<> struct read_impl<std::uint8_t> { static std::uint8_t read(std::istream& is) { return detail::read<8, std::uint8_t>(is); } };
		template<> struct read_impl<std::uint16_t> { static std::uint16_t read(std::istream& is) { return detail::read<16, std::uint16_t>(is); } };
		template<> struct read_impl<std::uint32_t> { static std::uint32_t read(std::istream& is) { return detail::read<32, std::uint32_t>(is); } };
		template<> struct read_impl<std::uint64_t> { static std::uint64_t read(std::istream& is) { return detail::read<64, std::uint64_t>(is); } };
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

		template<class T>
		struct write_impl;

		template<> struct write_impl<bool> { static void write(std::ostream& os, bool value) { detail::write<8>(os, std::uint8_t{ value }); } };
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

		template<class T>
		void write(std::ostream& os, T value)
		{
			return write_impl<T>::write(os, value);
		}

	} // io

} // bump
