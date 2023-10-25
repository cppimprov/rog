#include <bump_io.hpp>

#include <gtest/gtest.h>

namespace bump
{

	namespace io
	{
		
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
			static void write(std::ostream& os, test_struct const& value, std::endian endian)
			{
				write_32(os, value.a, endian);
				write_8(os, value.b, endian);
				write_32(os, value.c, endian);
				write_64(os, value.d, endian);
				write_8(os, value.e, endian);
			}
		};

		template<>
		struct read_impl<test_struct>
		{
			static test_struct read(std::istream& is, std::endian endian)
			{
				auto result = test_struct();

				result.a = read_32<std::uint32_t>(is, endian);
				result.b = read_8<bool>(is, endian);
				result.c = read_32<float>(is, endian);
				result.d = read_64<double>(is, endian);
				result.e = read_8<std::int8_t>(is, endian);

				return result;
			}
		};

		TEST(Test_bump_io_write, syntax)
		{
			auto const d = test_struct{ 0x12345678, true, 3.14f, 1.23456789, -123 };

			auto s = std::string();
			s.reserve(1024);

			auto os = std::ostringstream(std::move(s));

			write_struct(os, d, std::endian::big);

			auto is = std::istringstream(std::move(os.str()));

			auto const e = read_struct<test_struct>(is, std::endian::big);

			EXPECT_EQ(d, e);
		}

		// ok... test all the basic types!
		// also make sure that weird things don't compile (wrong size, etc.)

	} // io

} // bump
