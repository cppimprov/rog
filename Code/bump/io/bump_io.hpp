#pragma once

#include "bump_result.hpp"

#include <bit>
#include <span>

namespace bump
{

	namespace io
	{

		enum class io_err
		{
			output_buffer_too_small = 1,
		};

		inline result<void, io_err> copy_span(std::span<char const> src, std::span<char> dst)
		{
			if (dst.size() < src.size())
				return make_err(io_err::output_buffer_too_small);

			std::memcpy(dst.data(), src.data(), src.size());

			return make_ok();
		}
		
		namespace 
		{

			// alas std::make_unsigned_t<bool> is not defined :(
			template<class T>
			using get_unsigned_t = std::conditional_t<std::is_same_v<T, bool>, std::uint8_t, std::make_unsigned_t<T>>;

			template<std::size_t Bits, class T>
			result<void, io_err> write(std::span<char> dst, T value, std::endian e)
			{
				static_assert(std::is_integral_v<T>, "type T is not an integral type");
				static_assert(sizeof(T) * CHAR_BIT == Bits, "type T is not the correct size");

				auto bytes = std::bit_cast<get_unsigned_t<T>>(value);

				if (e != std::endian::native)
					bytes = std::byteswap(bytes);

				return copy_span({ reinterpret_cast<char const*>(&bytes), sizeof(bytes) }, dst);
			}

			template<std::size_t Bits, class T>
			result<T, io_err> read(std::span<char const> src, std::endian e)
			{
				static_assert(std::is_integral_v<T>, "type T is not an integral type");
				static_assert(sizeof(T) * CHAR_BIT == Bits, "type T is not the correct size");

				get_unsigned_t<T> bytes;
				auto copy_result = copy_span(src, { reinterpret_cast<char*>(&bytes), sizeof(bytes) });

				if (!copy_result.has_value())
					return make_err(copy_result.error());

				if (e != std::endian::native)
					bytes = std::byteswap(bytes);

				return make_ok(std::bit_cast<T>(bytes));
			}
		
		} // unnamed

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_8(std::span<char> dst, T value, std::endian endian) { return write<8>(dst, value, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_8_be(std::span<char> dst, T value) { return write_8(dst, value, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_8_le(std::span<char> dst, T value) { return write_8(dst, value, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_8_ne(std::span<char> dst, T value) { return write_8(dst, value, std::endian::native); }
		
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_16(std::span<char> dst, T value, std::endian endian) { return write<16>(dst, value, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_16_be(std::span<char> dst, T value) { return write_16(dst, value, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_16_le(std::span<char> dst, T value) { return write_16(dst, value, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_16_ne(std::span<char> dst, T value) { return write_16(dst, value, std::endian::native); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_32(std::span<char> dst, T value, std::endian endian) { return write<32>(dst, value, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_32_be(std::span<char> dst, T value) { return write_32(dst, value, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_32_le(std::span<char> dst, T value) { return write_32(dst, value, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_32_ne(std::span<char> dst, T value) { return write_32(dst, value, std::endian::native); }

		inline result<void, io_err> write_32(std::span<char> dst, float value, std::endian endian) { return write<32>(dst, std::bit_cast<std::uint32_t>(value), endian); }
		inline result<void, io_err> write_32_be(std::span<char> dst, float value) { return write_32(dst, value, std::endian::big); }
		inline result<void, io_err> write_32_le(std::span<char> dst, float value) { return write_32(dst, value, std::endian::little); }
		inline result<void, io_err> write_32_ne(std::span<char> dst, float value) { return write_32(dst, value, std::endian::native); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_64(std::span<char> dst, T value, std::endian endian) { return write<64>(dst, value, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_64_be(std::span<char> dst, T value) { return write_64(dst, value, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_64_le(std::span<char> dst, T value) { return write_64(dst, value, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<void, io_err> write_64_ne(std::span<char> dst, T value) { return write_64(dst, value, std::endian::native); }

		inline result<void, io_err> write_64(std::span<char> dst, double value, std::endian endian) { return write<64>(dst, std::bit_cast<std::uint64_t>(value), endian); }
		inline result<void, io_err> write_64_be(std::span<char> dst, double value) { return write_64(dst, value, std::endian::big); }
		inline result<void, io_err> write_64_le(std::span<char> dst, double value) { return write_64(dst, value, std::endian::little); }
		inline result<void, io_err> write_64_ne(std::span<char> dst, double value) { return write_64(dst, value, std::endian::native); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_8(std::span<char const> src, std::endian endian) { return read<8, T>(src, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_8_be(std::span<char const> src) { return read_8<T>(src, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_8_le(std::span<char const> src) { return read_8<T>(src, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_8_ne(std::span<char const> src) { return read_8<T>(src, std::endian::native); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_16(std::span<char const> src, std::endian endian) { return read<16, T>(src, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_16_be(std::span<char const> src) { return read_16<T>(src, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_16_le(std::span<char const> src) { return read_16<T>(src, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_16_ne(std::span<char const> src) { return read_16<T>(src, std::endian::native); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_32(std::span<char const> src, std::endian endian) { return read<32, T>(src, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_32_be(std::span<char const> src) { return read_32<T>(src, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_32_le(std::span<char const> src) { return read_32<T>(src, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_32_ne(std::span<char const> src) { return read_32<T>(src, std::endian::native); }

		inline result<float, io_err> read_32(std::span<char const> src, std::endian endian) { auto const f = read<32, std::uint32_t>(src, endian); if (!f.has_value()) return make_err(f.error()); return make_ok(std::bit_cast<float>(f.value())); }
		inline result<float, io_err> read_32_be(std::span<char const> src) { return read_32(src, std::endian::big); }
		inline result<float, io_err> read_32_le(std::span<char const> src) { return read_32(src, std::endian::little); }
		inline result<float, io_err> read_32_ne(std::span<char const> src) { return read_32(src, std::endian::native); }

		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_64(std::span<char const> src, std::endian endian) { return read<64, T>(src, endian); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_64_be(std::span<char const> src) { return read_64<T>(src, std::endian::big); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_64_le(std::span<char const> src) { return read_64<T>(src, std::endian::little); }
		template<class T, std::enable_if_t<std::is_integral_v<T>, void>> result<T, io_err> read_64_ne(std::span<char const> src) { return read_64<T>(src, std::endian::native); }

		inline result<double, io_err> read_64(std::span<char const> src, std::endian endian) { auto const d = read<64, std::uint64_t>(src, endian); if (!d.has_value()) return make_err(d.error()); return make_ok(std::bit_cast<double>(d.value())); }
		inline result<double, io_err> read_64_be(std::span<char const> src) { return read_64(src, std::endian::big); }
		inline result<double, io_err> read_64_le(std::span<char const> src) { return read_64(src, std::endian::little); }
		inline result<double, io_err> read_64_ne(std::span<char const> src) { return read_64(src, std::endian::native); }
		
	} // io

} // bump
