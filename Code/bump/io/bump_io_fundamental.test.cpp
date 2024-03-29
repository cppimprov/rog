#include <bump_io.hpp>

#include <gtest/gtest.h>

namespace bump
{

	namespace io
	{
		// note:
		// Many of these tests are written to be platform specific. They will
		// break on platforms with different sized fundamental types. If this 
		// happens, the tests should be adjusted for those platforms.
		
		TEST(Test_bump_io_read_write, bool)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, true);
			write(os, false);
			set_endian(os, std::endian::little);
			write(os, true);
			write(os, false);

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 4);
			EXPECT_EQ(s, (std::string{ '\x01', '\x00', '\x01', '\x00' }));

			auto is = std::istringstream(std::move(s));
			set_endian(is, std::endian::big);
			EXPECT_EQ(read<bool>(is), true);
			EXPECT_EQ(read<bool>(is), false);
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<bool>(is), true);
			EXPECT_EQ(read<bool>(is), false);
		}

		TEST(Test_bump_io_read_write, char)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, 'a');
			write(os, '_');
			set_endian(os, std::endian::little);
			write(os, '\x00');
			write(os, '\xff');

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 4);
			EXPECT_EQ(s, (std::string{ 'a', '_', '\x00', '\xff' }));

			auto is = std::istringstream(std::move(s));
			set_endian(os, std::endian::big);
			EXPECT_EQ(read<char>(is), 'a');
			EXPECT_EQ(read<char>(is), '_');
			set_endian(os, std::endian::little);
			EXPECT_EQ(read<char>(is), '\x00');
			EXPECT_EQ(read<char>(is), '\xff');
		}

		TEST(Test_bump_io_read_write, unsigned_char)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, unsigned char{ 0 });
			write(os, unsigned char{ 32 });
			set_endian(os, std::endian::little);
			write(os, unsigned char{ 128 });
			write(os, unsigned char{ 255 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 4);
			EXPECT_EQ(s, (std::string{ '\x00', '\x20', '\x80', '\xff' }));

			auto is = std::istringstream(std::move(s));
			set_endian(is, std::endian::big);
			EXPECT_EQ(read<unsigned char>(is), (unsigned char{ 0 }));
			EXPECT_EQ(read<unsigned char>(is), (unsigned char{ 32 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<unsigned char>(is), (unsigned char{ 128 }));
			EXPECT_EQ(read<unsigned char>(is), (unsigned char{ 255 }));
		}

		TEST(Test_bump_io_read_write, signed_char)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, signed char{ -128 });
			write(os, signed char{ -32 });
			write(os, signed char{ 0 });
			set_endian(os, std::endian::little);
			write(os, signed char{ 32 });
			write(os, signed char{ 127 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 5);
			EXPECT_EQ(s, (std::string{ '\x80', '\xe0', '\x00', '\x20', '\x7f' }));

			auto is = std::istringstream(std::move(s));
			set_endian(is, std::endian::big);
			EXPECT_EQ(read<signed char>(is), (signed char{ -128 }));
			EXPECT_EQ(read<signed char>(is), (signed char{ -32 }));
			EXPECT_EQ(read<signed char>(is), (signed char{ 0 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<signed char>(is), (signed char{ 32 }));
			EXPECT_EQ(read<signed char>(is), (signed char{ 127 }));
		}

		TEST(Test_bump_io_read_write, char8_t)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, char8_t{ 0 });
			write(os, char8_t{ 32 });
			set_endian(os, std::endian::little);
			write(os, char8_t{ 128 });
			write(os, char8_t{ 255 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 4);
			EXPECT_EQ(s, (std::string{ '\x00', '\x20', '\x80', '\xff' }));

			auto is = std::istringstream(std::move(s));

			// note: this build of gtest doesn't know about char8_t :(
			set_endian(is, std::endian::big);
			EXPECT_EQ((unsigned char)read<char8_t>(is), (unsigned char)(char8_t{ 0 }));
			EXPECT_EQ((unsigned char)read<char8_t>(is), (unsigned char)(char8_t{ 32 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ((unsigned char)read<char8_t>(is), (unsigned char)(char8_t{ 128 }));
			EXPECT_EQ((unsigned char)read<char8_t>(is), (unsigned char)(char8_t{ 255 }));
		}

		TEST(Test_bump_io_read_write, char16_t)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, char16_t{ 0 });
			write(os, char16_t{ 32 });
			write(os, char16_t{ 255 });
			write(os, char16_t{ 0x77ff });
			set_endian(os, std::endian::little);
			write(os, char16_t{ 32 });
			write(os, char16_t{ 255 });
			write(os, char16_t{ 0x77ff });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 14);
			EXPECT_EQ(s, (std::string{ 
				'\x00', '\x00',
				'\x00', '\x20',
				'\x00', '\xff',
				'\x77', '\xff',
				'\x20', '\x00',
				'\xff', '\x00',
				'\xff', '\x77'
			}));

			auto is = std::istringstream(std::move(s));

			// note: this build of gtest doesn't know about char16_t :(
			set_endian(is, std::endian::big);
			EXPECT_EQ((std::uint16_t)read<char16_t>(is), (std::uint16_t)(char16_t{ 0 }));
			EXPECT_EQ((std::uint16_t)read<char16_t>(is), (std::uint16_t)(char16_t{ 32 }));
			EXPECT_EQ((std::uint16_t)read<char16_t>(is), (std::uint16_t)(char16_t{ 255 }));
			EXPECT_EQ((std::uint16_t)read<char16_t>(is), (std::uint16_t)(char16_t{ 0x77ff }));
			set_endian(is, std::endian::little);
			EXPECT_EQ((std::uint16_t)read<char16_t>(is), (std::uint16_t)(char16_t{ 32 }));
			EXPECT_EQ((std::uint16_t)read<char16_t>(is), (std::uint16_t)(char16_t{ 255 }));
			EXPECT_EQ((std::uint16_t)read<char16_t>(is), (std::uint16_t)(char16_t{ 0x77ff }));
		}

		TEST(Test_bump_io_read_write, char32_t)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, char32_t{ 0 });
			write(os, char32_t{ 255 });
			write(os, char32_t{ 0x77ff });
			write(os, char32_t{ 0x77ff77ff });
			set_endian(os, std::endian::little);
			write(os, char32_t{ 255 });
			write(os, char32_t{ 0x77ff });
			write(os, char32_t{ 0x77ff77ff });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 28);
			EXPECT_EQ(s, (std::string{
				'\x00', '\x00', '\x00', '\x00',
				'\x00', '\x00', '\x00', '\xff',
				'\x00', '\x00', '\x77', '\xff',
				'\x77', '\xff', '\x77', '\xff',
				'\xff', '\x00', '\x00', '\x00',
				'\xff', '\x77', '\x00', '\x00',
				'\xff', '\x77', '\xff', '\x77'
			}));

			auto is = std::istringstream(std::move(s));

			// note: this build of gtest doesn't know about char32_t :(
			set_endian(is, std::endian::big);
			EXPECT_EQ((std::uint32_t)read<char32_t>(is), (std::uint32_t)(char32_t{ 0 }));
			EXPECT_EQ((std::uint32_t)read<char32_t>(is), (std::uint32_t)(char32_t{ 255 }));
			EXPECT_EQ((std::uint32_t)read<char32_t>(is), (std::uint32_t)(char32_t{ 0x77ff }));
			EXPECT_EQ((std::uint32_t)read<char32_t>(is), (std::uint32_t)(char32_t{ 0x77ff77ff }));
			set_endian(is, std::endian::little);
			EXPECT_EQ((std::uint32_t)read<char32_t>(is), (std::uint32_t)(char32_t{ 255 }));
			EXPECT_EQ((std::uint32_t)read<char32_t>(is), (std::uint32_t)(char32_t{ 0x77ff }));
			EXPECT_EQ((std::uint32_t)read<char32_t>(is), (std::uint32_t)(char32_t{ 0x77ff77ff }));
		}

		TEST(Test_bump_io_read_write, short_int)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, short{ -32768 });
			write(os, short{ -1 });
			write(os, short{ 0 });
			write(os, short{ 1 });
			write(os, short{ 32767 });
			set_endian(os, std::endian::little);
			write(os, short{ -32768 });
			write(os, short{ -1 });
			write(os, short{ 0 });
			write(os, short{ 1 });
			write(os, short{ 32767 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 20);
			EXPECT_EQ(s, (std::string{
				'\x80', '\x00',
				'\xff', '\xff',
				'\x00', '\x00',
				'\x00', '\x01',
				'\x7f', '\xff',
				'\x00', '\x80',
				'\xff', '\xff',
				'\x00', '\x00',
				'\x01', '\x00',
				'\xff', '\x7f'
			}));

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_EQ(read<short>(is), (short{ -32768 }));
			EXPECT_EQ(read<short>(is), (short{ -1 }));
			EXPECT_EQ(read<short>(is), (short{ 0 }));
			EXPECT_EQ(read<short>(is), (short{ 1 }));
			EXPECT_EQ(read<short>(is), (short{ 32767 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<short>(is), (short{ -32768 }));
			EXPECT_EQ(read<short>(is), (short{ -1 }));
			EXPECT_EQ(read<short>(is), (short{ 0 }));
			EXPECT_EQ(read<short>(is), (short{ 1 }));
			EXPECT_EQ(read<short>(is), (short{ 32767 }));
		}

		TEST(Test_bump_io_read_write, unsigned_short_int)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, unsigned short{ 0 });
			write(os, unsigned short{ 1 });
			write(os, unsigned short{ 32767 });
			write(os, unsigned short{ 32768 });
			write(os, unsigned short{ 65535 });
			set_endian(os, std::endian::little);
			write(os, unsigned short{ 0 });
			write(os, unsigned short{ 1 });
			write(os, unsigned short{ 32767 });
			write(os, unsigned short{ 32768 });
			write(os, unsigned short{ 65535 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 20);
			EXPECT_EQ(s, (std::string{
				'\x00', '\x00',
				'\x00', '\x01',
				'\x7f', '\xff',
				'\x80', '\x00',
				'\xff', '\xff',
				'\x00', '\x00',
				'\x01', '\x00',
				'\xff', '\x7f',
				'\x00', '\x80',
				'\xff', '\xff'
			}));

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 0 }));
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 1 }));
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 32767 }));
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 32768 }));
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 65535 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 0 }));
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 1 }));
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 32767 }));
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 32768 }));
			EXPECT_EQ(read<unsigned short>(is), (unsigned short{ 65535 }));
		}

		TEST(Test_bump_io_read_write, int)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, int{ -2147483647 - 1 });
			write(os, int{ -1 });
			write(os, int{ 0 });
			write(os, int{ 1 });
			write(os, int{ 2147483647 });
			set_endian(os, std::endian::little);
			write(os, int{ -2147483647 - 1 });
			write(os, int{ -1 });
			write(os, int{ 0 });
			write(os, int{ 1 });
			write(os, int{ 2147483647 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 40);
			EXPECT_EQ(s, (std::string{
				'\x80', '\x00', '\x00', '\x00',
				'\xff', '\xff', '\xff', '\xff',
				'\x00', '\x00', '\x00', '\x00',
				'\x00', '\x00', '\x00', '\x01',
				'\x7f', '\xff', '\xff', '\xff',
				'\x00', '\x00', '\x00', '\x80',
				'\xff', '\xff', '\xff', '\xff',
				'\x00', '\x00', '\x00', '\x00',
				'\x01', '\x00', '\x00', '\x00',
				'\xff', '\xff', '\xff', '\x7f'
			}));

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_EQ(read<int>(is), (int{ -2147483647 - 1 }));
			EXPECT_EQ(read<int>(is), (int{ -1 }));
			EXPECT_EQ(read<int>(is), (int{ 0 }));
			EXPECT_EQ(read<int>(is), (int{ 1 }));
			EXPECT_EQ(read<int>(is), (int{ 2147483647 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<int>(is), (int{ -2147483647 - 1 }));
			EXPECT_EQ(read<int>(is), (int{ -1 }));
			EXPECT_EQ(read<int>(is), (int{ 0 }));
			EXPECT_EQ(read<int>(is), (int{ 1 }));
			EXPECT_EQ(read<int>(is), (int{ 2147483647 }));
		}

		TEST(Test_bump_io_read_write, unsigned_int)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, unsigned int{ 0 });
			write(os, unsigned int{ 1 });
			write(os, unsigned int{ 2147483647 });
			write(os, unsigned int{ 2147483648 });
			write(os, unsigned int{ 4294967295 });
			set_endian(os, std::endian::little);
			write(os, unsigned int{ 0 });
			write(os, unsigned int{ 1 });
			write(os, unsigned int{ 2147483647 });
			write(os, unsigned int{ 2147483648 });
			write(os, unsigned int{ 4294967295 });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 40);
			EXPECT_EQ(s, (std::string{
				'\x00', '\x00', '\x00', '\x00',
				'\x00', '\x00', '\x00', '\x01',
				'\x7f', '\xff', '\xff', '\xff',
				'\x80', '\x00', '\x00', '\x00',
				'\xff', '\xff', '\xff', '\xff',
				'\x00', '\x00', '\x00', '\x00',
				'\x01', '\x00', '\x00', '\x00',
				'\xff', '\xff', '\xff', '\x7f',
				'\x00', '\x00', '\x00', '\x80',
				'\xff', '\xff', '\xff', '\xff'
			}));

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 0 }));
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 1 }));
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 2147483647 }));
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 2147483648 }));
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 4294967295 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 0 }));
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 1 }));
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 2147483647 }));
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 2147483648 }));
			EXPECT_EQ(read<unsigned int>(is), (unsigned int{ 4294967295 }));
		}

		TEST(Test_bump_io_read_write, float)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, float{ std::numeric_limits<float>::max() });
			write(os, float{ -std::numeric_limits<float>::max() });
			write(os, float{ std::numeric_limits<float>::min() });
			write(os, float{ -std::numeric_limits<float>::min() });
			write(os, float{ 0.15625f });
			set_endian(os, std::endian::little);
			write(os, float{ std::numeric_limits<float>::max() });
			write(os, float{ -std::numeric_limits<float>::max() });
			write(os, float{ std::numeric_limits<float>::min() });
			write(os, float{ -std::numeric_limits<float>::min() });
			write(os, float{ 0.15625f });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 40);
			EXPECT_EQ(s, (std::string{
				'\x7f', '\x7f', '\xff', '\xff',
				'\xff', '\x7f', '\xff', '\xff',
				'\x00', '\x80', '\x00', '\x00',
				'\x80', '\x80', '\x00', '\x00',
				'\x3e', '\x20', '\x00', '\x00',
				'\xff', '\xff', '\x7f', '\x7f',
				'\xff', '\xff', '\x7f', '\xff',
				'\x00', '\x00', '\x80', '\x00',
				'\x00', '\x00', '\x80', '\x80',
				'\x00', '\x00', '\x20', '\x3e'
			}));

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_EQ(read<float>(is), (float{ std::numeric_limits<float>::max() }));
			EXPECT_EQ(read<float>(is), (float{ -std::numeric_limits<float>::max() }));
			EXPECT_EQ(read<float>(is), (float{ std::numeric_limits<float>::min() }));
			EXPECT_EQ(read<float>(is), (float{ -std::numeric_limits<float>::min() }));
			EXPECT_EQ(read<float>(is), (float{ 0.15625f }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<float>(is), (float{ std::numeric_limits<float>::max() }));
			EXPECT_EQ(read<float>(is), (float{ -std::numeric_limits<float>::max() }));
			EXPECT_EQ(read<float>(is), (float{ std::numeric_limits<float>::min() }));
			EXPECT_EQ(read<float>(is), (float{ -std::numeric_limits<float>::min() }));
			EXPECT_EQ(read<float>(is), (float{ 0.15625f }));
		}

		TEST(Test_bump_io_read_write, float_inf)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, float{ std::numeric_limits<float>::infinity() });
			write(os, float{ -std::numeric_limits<float>::infinity() });
			set_endian(os, std::endian::little);
			write(os, float{ std::numeric_limits<float>::infinity() });
			write(os, float{ -std::numeric_limits<float>::infinity() });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 16);
			EXPECT_EQ(s, (std::string{
				'\x7f', '\x80', '\x00', '\x00',
				'\xff', '\x80', '\x00', '\x00',
				'\x00', '\x00', '\x80', '\x7f',
				'\x00', '\x00', '\x80', '\xff'
			}));

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_EQ(read<float>(is), (float{ std::numeric_limits<float>::infinity() }));
			EXPECT_EQ(read<float>(is), (float{ -std::numeric_limits<float>::infinity() }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<float>(is), (float{ std::numeric_limits<float>::infinity() }));
			EXPECT_EQ(read<float>(is), (float{ -std::numeric_limits<float>::infinity() }));
		}

		TEST(Test_bump_io_read_write, float_nan)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, float{ std::numeric_limits<float>::quiet_NaN() });
			set_endian(os, std::endian::little);
			write(os, float{ std::numeric_limits<float>::quiet_NaN() });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 8);

			// note: we don't check the string representation
			// IEEE 754 says that NaN's should have all exponent bits and at
			// least one mantissa bit set...
			// but we really just care that we get a NaN back afterwards!

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_TRUE(std::isnan(read<float>(is)));
			set_endian(is, std::endian::little);
			EXPECT_TRUE(std::isnan(read<float>(is)));
		}

		TEST(Test_bump_io_read_write, long_long_int)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, long long{ -9223372036854775807LL - 1LL });
			write(os, long long{ -1 });
			write(os, long long{ 0 });
			write(os, long long{ 1 });
			write(os, long long{ 9223372036854775807LL });
			set_endian(os, std::endian::little);
			write(os, long long{ -9223372036854775807LL - 1LL });
			write(os, long long{ -1 });
			write(os, long long{ 0 });
			write(os, long long{ 1 });
			write(os, long long{ 9223372036854775807LL });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 80);
			EXPECT_EQ(s, (std::string{
				'\x80', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01',
				'\x7f', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x80',
				'\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\x01', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\x7f'
			}));

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_EQ(read<long long>(is), (long long{ -9223372036854775807LL - 1LL }));
			EXPECT_EQ(read<long long>(is), (long long{ -1 }));
			EXPECT_EQ(read<long long>(is), (long long{ 0 }));
			EXPECT_EQ(read<long long>(is), (long long{ 1 }));
			EXPECT_EQ(read<long long>(is), (long long{ 9223372036854775807LL }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<long long>(is), (long long{ -9223372036854775807LL - 1LL }));
			EXPECT_EQ(read<long long>(is), (long long{ -1 }));
			EXPECT_EQ(read<long long>(is), (long long{ 0 }));
			EXPECT_EQ(read<long long>(is), (long long{ 1 }));
			EXPECT_EQ(read<long long>(is), (long long{ 9223372036854775807LL }));
		}

		TEST(Test_bump_io_read_write, unsigned_long_long_int)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, unsigned long long{ 0 });
			write(os, unsigned long long{ 1 });
			write(os, unsigned long long{ 9223372036854775807LL });
			write(os, unsigned long long{ 9223372036854775808ULL });
			write(os, unsigned long long{ 18446744073709551615ULL });
			set_endian(os, std::endian::little);
			write(os, unsigned long long{ 0 });
			write(os, unsigned long long{ 1 });
			write(os, unsigned long long{ 9223372036854775807LL });
			write(os, unsigned long long{ 9223372036854775808ULL });
			write(os, unsigned long long{ 18446744073709551615ULL });

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 80);
			EXPECT_EQ(s, (std::string{
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01',
				'\x7f', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',
				'\x80', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\x01', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\x7f',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x80',
				'\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff'
			}));

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 0 }));
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 1 }));
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 9223372036854775807LL }));
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 9223372036854775808ULL }));
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 18446744073709551615ULL }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 0 }));
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 1 }));
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 9223372036854775807LL }));
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 9223372036854775808ULL }));
			EXPECT_EQ(read<unsigned long long>(is), (unsigned long long{ 18446744073709551615ULL }));
		}

		TEST(Test_bump_io_read_write, double)
		{
			auto os = std::ostringstream();
			set_endian(os, std::endian::big);
			write(os, double{ std::numeric_limits<double>::max() });
			write(os, double{ -std::numeric_limits<double>::max() });
			write(os, double{ std::numeric_limits<double>::min() });
			write(os, double{ -std::numeric_limits<double>::min() });
			write(os, double{ 0.15625 });
			set_endian(os, std::endian::little);
			write(os, double{ std::numeric_limits<double>::max() });
			write(os, double{ -std::numeric_limits<double>::max() });
			write(os, double{ std::numeric_limits<double>::min() });
			write(os, double{ -std::numeric_limits<double>::min() });
			write(os, double{ 0.15625 });

			auto const x = std::bit_cast<std::uint64_t>(0.15625);
			(void)x;

			auto s = std::move(os.str());

			EXPECT_EQ(s.size(), 80);
			EXPECT_EQ(s, (std::string{
				'\x7f', '\xef', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',
				'\xff', '\xef', '\xff', '\xff', '\xff', '\xff', '\xff', '\xff',
				'\x00', '\x10', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\x80', '\x10', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\x3f', '\xc4', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
				'\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xef', '\x7f',
				'\xff', '\xff', '\xff', '\xff', '\xff', '\xff', '\xef', '\xff',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x10', '\x00',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x10', '\x80',
				'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\xc4', '\x3f'
			}));

			auto is = std::istringstream(std::move(s));

			set_endian(is, std::endian::big);
			EXPECT_EQ(read<double>(is), (double{ std::numeric_limits<double>::max() }));
			EXPECT_EQ(read<double>(is), (double{ -std::numeric_limits<double>::max() }));
			EXPECT_EQ(read<double>(is), (double{ std::numeric_limits<double>::min() }));
			EXPECT_EQ(read<double>(is), (double{ -std::numeric_limits<double>::min() }));
			EXPECT_EQ(read<double>(is), (double{ 0.15625 }));
			set_endian(is, std::endian::little);
			EXPECT_EQ(read<double>(is), (double{ std::numeric_limits<double>::max() }));
			EXPECT_EQ(read<double>(is), (double{ -std::numeric_limits<double>::max() }));
			EXPECT_EQ(read<double>(is), (double{ std::numeric_limits<double>::min() }));
			EXPECT_EQ(read<double>(is), (double{ -std::numeric_limits<double>::min() }));
			EXPECT_EQ(read<double>(is), (double{ 0.15625 }));
		}

		struct test_struct
		{
			std::uint32_t a;
			bool b;
			float c;
			double d;
			std::int8_t e;

			bool operator==(test_struct const& rhs) const = default;
		};

		template<>
		struct write_impl<test_struct>
		{
			static void write(std::ostream& os, test_struct const& value)
			{
				io::write(os, value.a);
				io::write(os, value.b);
				io::write(os, value.c);
				io::write(os, value.d);
				io::write(os, value.e);
			}
		};

		template<>
		struct read_impl<test_struct>
		{
			static test_struct read(std::istream& is)
			{
				auto result = test_struct();

				result.a = io::read<decltype(result.a)>(is);
				result.b = io::read<decltype(result.b)>(is);
				result.c = io::read<decltype(result.c)>(is);
				result.d = io::read<decltype(result.d)>(is);
				result.e = io::read<decltype(result.e)>(is);

				return result;
			}
		};

		TEST(Test_bump_io_write, syntax)
		{
			auto const d = test_struct{ 0x12345678, true, 3.14f, 1.23456789, -123 };

			auto s = std::string();
			s.reserve(1024);

			auto os = std::ostringstream(std::move(s));

			write(os, d);

			auto is = std::istringstream(std::move(os.str()));

			auto const e = read<test_struct>(is);

			EXPECT_EQ(d, e);
		}

		// 2.5 test std container support
		// 3. glm support
		// 3.5 test math support

	} // io

} // bump
