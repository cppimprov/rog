#include <bump_io.hpp>

#include <gtest/gtest.h>

namespace bump
{

	namespace io
	{
	
		TEST(Test_bump_io_stream_endian, default_is_big)
		{
			auto os = std::ostringstream();
			EXPECT_EQ(get_endian(os), std::endian::big);
			auto is = std::istringstream();
			EXPECT_EQ(get_endian(is), std::endian::big);
		}

		TEST(Test_bump_io_stream_endian, change_per_stream)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::little);
			EXPECT_EQ(get_endian(os), std::endian::little);
			auto is = std::istringstream();
			EXPECT_EQ(get_endian(is), std::endian::big);
			set_endian(is, std::endian::big);
			EXPECT_EQ(get_endian(os), std::endian::little);
			EXPECT_EQ(get_endian(is), std::endian::big);
		}

		TEST(Test_bump_io_read_write, endian)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, std::uint16_t{ 0x1234 });
			set_endian(os, std::endian::little);
			write(os, std::uint16_t{ 0x1234 });
			set_endian(os, std::endian::native);
			write(os, std::uint16_t{ 0x1234 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 6);
			EXPECT_EQ(s.substr(0, 4), (std::string{ '\x12', '\x34', '\x34', '\x12' }));

			if constexpr(std::endian::native == std::endian::big)
				EXPECT_EQ(s.substr(4, 2), (std::string{ '\x12', '\x34' }));
			else
				EXPECT_EQ(s.substr(4, 2), (std::string{ '\x34', '\x12' }));

			auto is = std::istringstream(std::move(s));
			set_endian(is, std::endian::big);
			EXPECT_EQ(read<std::uint16_t>(is), (std::uint16_t{ 0x1234 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<std::uint16_t>(is), (std::uint16_t{ 0x1234 }));
			set_endian(is, std::endian::native);
			EXPECT_EQ(read<std::uint16_t>(is), (std::uint16_t{ 0x1234 }));
		}
	
	} // io

} // bump
