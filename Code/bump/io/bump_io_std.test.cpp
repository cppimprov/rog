#include <bump_io.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <utility>

namespace bump
{

	namespace io
	{

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
			// so that we can test reading containers that are too large for the platform
			size_type max_size() const noexcept { return 1; }
		};

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

			auto is = std::istringstream(std::move(s));
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

			auto is = std::istringstream(std::move(s));
			set_endian(is, std::endian::big);
			EXPECT_EQ(read<map_t>(is), (map_t{ { std::uint8_t{ 0x77 }, std::uint16_t{ 0x1234 } }, { std::uint8_t{ 0xFF }, std::uint16_t{ 0x1359 } } }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<map_t>(is), (map_t{ { std::uint8_t{ 0x77 }, std::uint16_t{ 0x1234 } }, { std::uint8_t{ 0xFF }, std::uint16_t{ 0x1359 } } }));
		}

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

			auto is = std::istringstream(std::move(s));
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

			auto is = std::istringstream(std::move(s));
			is.exceptions(std::ios::failbit | std::ios::badbit);
			set_endian(is, std::endian::big);
			EXPECT_THROW(read<map_t>(is), std::ios::failure);
		}

#pragma endregion

#pragma region std::string

		TEST(Test_bump_io_std, string)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write<std::string>(os, "Hello, world!");
			set_endian(os, std::endian::little);
			write<std::string>(os, "Hello, world!");

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 2 * (8 + 13));
			EXPECT_EQ(s, (std::string{
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x0D',
				'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!',
				'\x0D', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!'
			}));

			auto is = std::istringstream(std::move(s));
			set_endian(is, std::endian::big);
			EXPECT_EQ(read<std::string>(is), "Hello, world!");
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<std::string>(is), "Hello, world!");
		}

		TEST(Test_bump_io_std, u_string)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write<std::u8string>(os, u8"Hello, world!");
			write<std::u16string>(os, u"Hello!");

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), (8 + 13) + (8 + 6 * 2));
			EXPECT_EQ(s, (std::string{
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x0D',
				'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x06',
				'\x00', 'H', '\x00', 'e', '\x00', 'l', '\x00', 'l', '\x00', 'o', '\x00', '!',
			}));

			auto is = std::istringstream(std::move(s));
			set_endian(is, std::endian::big);
			EXPECT_TRUE((read<std::u8string>(is) == u8"Hello, world!"));
			EXPECT_TRUE((read<std::u16string>(is) == u"Hello!"));
		}

		TEST(Test_bump_io_std, string_view)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write<std::string_view>(os, "Hello, world!");
			write<std::u16string_view>(os, u"Hello!");

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), (8 + 13) + (8 + 6 * 2));

			auto is = std::istringstream(std::move(s));
			set_endian(is, std::endian::big);

			// note: no corresponding read<std::string_view> function (obvs)
			EXPECT_TRUE((read<std::string>(is) == "Hello, world!"));
			EXPECT_TRUE((read<std::u16string>(is) == u"Hello!"));
		}

		TEST(Test_bump_io_std, string_too_large)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			// note: small string optimization means this string has to be much
			// longer than just 2 chars, even if the allocator max_size() returns 1
			write<std::string>(os, "Hello, world! 1234567890");

			auto s = std::move(os.str());
			EXPECT_EQ(s.size(), 8 + 24);

			auto is = std::istringstream(std::move(s));
			is.exceptions(std::ios::failbit | std::ios::badbit);
			set_endian(is, std::endian::big);
			using string_t = std::basic_string<char, std::char_traits<char>, alloc<char>>;
			EXPECT_THROW(read<string_t>(is), std::ios::failure);
		}

#pragma endregion

#pragma region std::vector

		TEST(Test_bump_io_std, vector)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write<std::vector<std::uint16_t>>(os, { 0x1234, 0x5678 });
			set_endian(os, std::endian::little);
			write<std::vector<std::uint16_t>>(os, { 0x1234, 0x5678 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 2 * (8 + 2 * 2));
			EXPECT_EQ(s, (std::string{
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x02',
				'\x12', '\x34', '\x56', '\x78',
				'\x02', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\x34', '\x12', '\x78', '\x56'
			}));

			auto is = std::istringstream(std::move(s));
			set_endian(is, std::endian::big);
			EXPECT_EQ(read<std::vector<std::uint16_t>>(is), (std::vector<std::uint16_t>{ 0x1234, 0x5678 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<std::vector<std::uint16_t>>(is), (std::vector<std::uint16_t>{ 0x1234, 0x5678 }));
		}

		TEST(Test_bump_io_std, vector_too_large)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write<std::vector<std::uint16_t>>(os, { 0x1234, 0x5678 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 8 + 2 * 2);

			auto is = std::istringstream(std::move(s));
			is.exceptions(std::ios::failbit | std::ios::badbit);
			set_endian(is, std::endian::big);
			using vector_t = std::vector<std::uint16_t, alloc<std::uint16_t>>;
			EXPECT_THROW(read<vector_t>(is), std::ios::failure);
		}

#pragma endregion

	} // io

} // bump
