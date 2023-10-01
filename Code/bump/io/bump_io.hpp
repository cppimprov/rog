#pragma once

#include <bit>
#include <iostream>

namespace bump
{

	namespace io
	{
	
		inline void write_data(std::ostream& os, char const* data, std::size_t size) { os.write(data, size); }
		inline void read_data(std::istream& is, char* data, std::size_t size) { is.read(data, size); }

		namespace 
		{

			// alas std::make_unsigned_t<bool> is not defined :(
			template<class T>
			using get_unsigned_t = std::conditional_t<std::is_same_v<T, bool>, std::uint8_t, std::make_unsigned_t<T>>;

			template<std::size_t Bits, class T>
			void write(std::ostream& os, T value, std::endian e)
			{
				static_assert(std::is_integral_v<T>, "T is not an integral type");
				static_assert(sizeof(T) * CHAR_BIT == Bits, "T does not have the correct size");

				auto bytes = std::bit_cast<get_unsigned_t<T>>(value);

				if (e != std::endian::native)
					bytes = std::byteswap(bytes);

				write_data(os, reinterpret_cast<char const*>(&bytes), sizeof(bytes));
			}

			template<std::size_t Bits, class T>
			T read(std::istream& is, std::endian e)
			{
				static_assert(std::is_integral_v<T>, "T is not an integral type");
				static_assert(sizeof(T) * CHAR_BIT == Bits, "T does not have the correct size");

				get_unsigned_t<T> bytes;
				read_data(is, reinterpret_cast<char*>(&bytes), sizeof(bytes));

				if (e != std::endian::native)
					bytes = std::byteswap(bytes);

				return std::bit_cast<T>(bytes);
			}
		
		} // unnamed

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_8(std::ostream& os, T value, std::endian endian) { write<8>(os, value, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_8_be(std::ostream& os, T value) { write_8(os, value, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_8_le(std::ostream& os, T value) { write_8(os, value, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_8_ne(std::ostream& os, T value) { write_8(os, value, std::endian::native); }
		
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_16(std::ostream& os, T value, std::endian endian) { write<16>(os, value, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_16_be(std::ostream& os, T value) { write_16(os, value, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_16_le(std::ostream& os, T value) { write_16(os, value, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_16_ne(std::ostream& os, T value) { write_16(os, value, std::endian::native); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_32(std::ostream& os, T value, std::endian endian) { write<32>(os, value, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_32_be(std::ostream& os, T value) { write_32(os, value, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_32_le(std::ostream& os, T value) { write_32(os, value, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_32_ne(std::ostream& os, T value) { write_32(os, value, std::endian::native); }

		inline void write_32(std::ostream& os, float value, std::endian endian) { write_32(os, std::bit_cast<std::uint32_t>(value), endian); }
		inline void write_32_be(std::ostream& os, float value) { write_32_be(os, std::bit_cast<std::uint32_t>(value)); }
		inline void write_32_le(std::ostream& os, float value) { write_32_le(os, std::bit_cast<std::uint32_t>(value)); }
		inline void write_32_ne(std::ostream& os, float value) { write_32_ne(os, std::bit_cast<std::uint32_t>(value)); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_64(std::ostream& os, T value, std::endian endian) { write<64>(os, value, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_64_be(std::ostream& os, T value) { write_64(os, value, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_64_le(std::ostream& os, T value) { write_64(os, value, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> void write_64_ne(std::ostream& os, T value) { write_64(os, value, std::endian::native); }

		inline void write_64(std::ostream& os, double value, std::endian endian) { write_64(os, std::bit_cast<std::uint64_t>(value), endian); }
		inline void write_64_be(std::ostream& os, double value) { write_64_be(os, std::bit_cast<std::uint64_t>(value)); }
		inline void write_64_le(std::ostream& os, double value) { write_64_le(os, std::bit_cast<std::uint64_t>(value)); }
		inline void write_64_ne(std::ostream& os, double value) { write_64_ne(os, std::bit_cast<std::uint64_t>(value)); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_8(std::istream& is, std::endian endian) { return read<8, T>(is, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_8_be(std::istream& is) { return read_8<T>(is, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_8_le(std::istream& is) { return read_8<T>(is, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_8_ne(std::istream& is) { return read_8<T>(is, std::endian::native); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_16(std::istream& is, std::endian endian) { return read<16, T>(is, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_16_be(std::istream& is) { return read_16<T>(is, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_16_le(std::istream& is) { return read_16<T>(is, std::endian::little); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_32(std::istream& is, std::endian endian) { return read<32, T>(is, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_32_be(std::istream& is) { return read_32<T>(is, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_32_le(std::istream& is) { return read_32<T>(is, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_32_ne(std::istream& is) { return read_32<T>(is, std::endian::native); }

		inline float read_32(std::istream& is, std::endian endian) { return std::bit_cast<float>(read_32<std::uint32_t>(is, endian)); }
		inline float read_32_be(std::istream& is) { return std::bit_cast<float>(read_32_be<std::uint32_t>(is)); }
		inline float read_32_le(std::istream& is) { return std::bit_cast<float>(read_32_le<std::uint32_t>(is)); }
		inline float read_32_ne(std::istream& is) { return std::bit_cast<float>(read_32_ne<std::uint32_t>(is)); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_64(std::istream& is, std::endian endian) { return read<64, T>(is, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_64_be(std::istream& is) { return read_64<T>(is, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_64_le(std::istream& is) { return read_64<T>(is, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> T read_64_ne(std::istream& is) { return read_64<T>(is, std::endian::native); }

		inline double read_64(std::istream& is, std::endian endian) { return std::bit_cast<double>(read_64<std::uint64_t>(is, endian)); }
		inline double read_64_be(std::istream& is) { return std::bit_cast<double>(read_64_be<std::uint64_t>(is)); }
		inline double read_64_le(std::istream& is) { return std::bit_cast<double>(read_64_le<std::uint64_t>(is)); }
		inline double read_64_ne(std::istream& is) { return std::bit_cast<double>(read_64_ne<std::uint64_t>(is)); }
		
	} // io

} // bump
