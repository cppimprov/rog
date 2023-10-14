#include <bump_io.hpp>

#include <gtest/gtest.h>

namespace bump
{

	namespace io
	{

		TEST(Test_bump_io_copy_span, empty_dst_empty_src)
		{
			auto const src = std::span<char const>{};
			auto dst = std::span<char>{};
			auto const result = copy_span(src, dst);
			EXPECT_TRUE(result.has_value());
		}
	
		TEST(Test_bump_io_copy_span, dst_too_small)
		{
			auto const src = std::span<char const>{ "hello world" };
			auto dst = std::span<char>{};
			auto const result = copy_span(src, dst);
			EXPECT_FALSE(result.has_value());
			EXPECT_EQ(result.error(), io_err::output_buffer_too_small);
		}

		TEST(Test_bump_io_copy_span, same_size_src_dst)
		{
			constexpr auto src = std::span<char const>{ "hello world" };
			char out[std::size(src)]{ 0 };
			auto dst = std::span<char>(out, std::size(out));
			ASSERT_TRUE(std::size(src) == std::size(dst));
			auto const result = copy_span(src, dst);
			EXPECT_TRUE(result.has_value());
			EXPECT_TRUE(std::equal(std::begin(src), std::end(src), std::begin(dst)));
		}

		TEST(Test_bump_io_copy_span, smaller_src)
		{
			constexpr auto src = std::span<char const>{ "hello world" };
			char out[std::size(src) + 10]{ 0 };
			auto dst = std::span<char>(out, std::size(out));
			ASSERT_TRUE(std::size(src) < std::size(dst));
			auto const result = copy_span(src, dst);
			EXPECT_TRUE(result.has_value());
			EXPECT_TRUE(std::equal(std::begin(src), std::end(src), std::begin(dst)));
		}

		// TODO: adjust the dst span after writing? or return number of bytes written?
		// write some usage code and see what feels better
	
	} // io

} // bump
