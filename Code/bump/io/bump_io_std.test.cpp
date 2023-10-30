#include <bump_io.hpp>

#include <gtest/gtest.h>

namespace bump
{

	namespace io
	{

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

		// map
			// simple
			// read too large? (set max_size with custom allocator class?)
			// duplicate keys  (custom write invalid data)

		// string
		// vector

	} // io

} // bump
