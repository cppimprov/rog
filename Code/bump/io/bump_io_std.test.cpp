#include <bump_io.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <utility>

namespace bump
{

	namespace io
	{

#pragma region std::pair

		TEST(Test_bump_io_std, pair)
		{
			using pair_t = std::pair<std::uint8_t, std::uint16_t>;

			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write<pair_t>(os, { std::uint8_t{ 0xFF }, std::uint16_t{ 0x1234 } });
			set_endian(os, std::endian::little);
			write<pair_t>(os, { std::uint8_t{ 0xFF }, std::uint16_t{ 0x1234 } });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 6);
			EXPECT_EQ(s, (std::string("\xFF\x12\x34\xFF\x34\x12")));

			auto is = std::istringstream(s);
			set_endian(is, std::endian::big);
			EXPECT_EQ(read<pair_t>(is), (pair_t{ std::uint8_t{ 0xFF }, std::uint16_t{ 0x1234 } }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<pair_t>(is), (pair_t{ std::uint8_t{ 0xFF }, std::uint16_t{ 0x1234 } }));
		}

#pragma endregion

#pragma region std::map

		TEST(Test_bump_io_std, map)
		{
			using map_t = std::map<std::uint8_t, std::uint16_t>;

			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write<map_t>(os, { { std::uint8_t{ 0x77 }, std::uint16_t{ 0x1234 } }, { std::uint8_t{ 0xFF }, std::uint16_t{ 0x1359 } } });
			set_endian(os, std::endian::little);
			write<map_t>(os, { { std::uint8_t{ 0x77 }, std::uint16_t{ 0x1234 } }, { std::uint8_t{ 0xFF }, std::uint16_t{ 0x1359 } } });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 28);
			EXPECT_EQ(s, (std::string{
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x02',
				'\x77', '\x12', '\x34',
				'\xFF', '\x13', '\x59',
				'\x02', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\x77', '\x34', '\x12',
				'\xFF', '\x59', '\x13'
			}));

			auto is = std::istringstream(s);
			set_endian(is, std::endian::big);
			EXPECT_EQ(read<map_t>(is), (map_t{ { std::uint8_t{ 0x77 }, std::uint16_t{ 0x1234 } }, { std::uint8_t{ 0xFF }, std::uint16_t{ 0x1359 } } }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<map_t>(is), (map_t{ { std::uint8_t{ 0x77 }, std::uint16_t{ 0x1234 } }, { std::uint8_t{ 0xFF }, std::uint16_t{ 0x1359 } } }));
		}

		template <typename T>
		class alloc
		{
		public:

			using value_type = T;
			using pointer = T*;
			using size_type = std::size_t;
			using is_always_equal = std::true_type;

			alloc() = default;

			template <typename U>
			alloc(alloc<U> const &) { }

			pointer allocate(size_type n)
			{
				auto p = static_cast<pointer>(malloc(sizeof(T) * n));
				if (!p) throw std::bad_alloc();
				return p;
			}

			void deallocate(pointer p, size_type)
			{
				free(p);
			}

			// we just wanted to do this!
			// so that we can test reading maps that are too large for the platform
			size_type max_size() const noexcept { return 1; }
		};

		TEST(Test_bump_io_std, map_too_large)
		{
			using map_t = std::map<
				std::uint8_t, std::uint16_t, 
				std::less<std::uint8_t>, 
				alloc<std::pair<std::uint8_t const, std::uint16_t>>
			>;

			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write<std::uint64_t>(os, 2);
			write<std::pair<std::uint8_t const, std::uint16_t>>(os, { std::uint8_t{ 0x00 }, std::uint16_t{ 0x1234 } });
			write<std::pair<std::uint8_t const, std::uint16_t>>(os, { std::uint8_t{ 0x01 }, std::uint16_t{ 0x1234 } });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 8 + 2 * 3);

			auto is = std::istringstream(s);
			is.exceptions(std::ios::failbit | std::ios::badbit);
			set_endian(is, std::endian::big);
			EXPECT_THROW(read<map_t>(is), std::ios::failure);
		}

		TEST(Test_bump_io_std, map_duplicate_key)
		{
			using map_t = std::map<std::uint8_t, std::uint16_t>;

			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write<std::uint64_t>(os, 2);
			write<std::pair<std::uint8_t const, std::uint16_t>>(os, { std::uint8_t{ 0x00 }, std::uint16_t{ 0x1234 } });
			write<std::pair<std::uint8_t const, std::uint16_t>>(os, { std::uint8_t{ 0x00 }, std::uint16_t{ 0x1359 } });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 8 + 2 * 3);

			auto is = std::istringstream(s);
			is.exceptions(std::ios::failbit | std::ios::badbit);
			set_endian(is, std::endian::big);
			EXPECT_THROW(read<map_t>(is), std::ios::failure);
		}

#pragma endregion

#pragma region std::string

		// ...

#pragma endregion

#pragma region std::vector

		// ...

#pragma endregion

	} // io

} // bump
