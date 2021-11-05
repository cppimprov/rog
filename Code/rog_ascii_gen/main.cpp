
#include <bump_font.hpp>
#include <bump_load_image.hpp>

#include <iostream>

namespace rog_ascii
{

	bump::image<std::uint8_t> render_ascii_tiles(bump::font::ft_context const& ft_context, bump::font::font_asset const& font, glm::i32vec2 tile_size)
	{
		using namespace std::string_literals;

		// 256 characters encoded as utf-8.
		// control characters (0 to 31 and some chars >127) are encoded as nulls.
		// characters from 128 to 255 are extended ascii (CP 1251) converted to utf-8.
		auto const chars_utf8 = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\0€\0‚ƒ„…†‡ˆ‰Š‹Œ\0Ž\0\0‘’“”•–—˜™š›œ\0žŸ ¡¢£¤¥¦§¨©ª«¬\0®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"s;

		auto hb_shaper = bump::font::hb_shaper(HB_DIRECTION_LTR, HB_SCRIPT_LATIN, hb_language_from_string("en", -1));
		hb_shaper.shape(font.m_hb_font.get_handle(), chars_utf8);

		auto glyphs = render_glyphs(ft_context, font.m_ft_font, font.m_hb_font, hb_shaper);

		if (glyphs.size() != 256)
		{
			std::cerr << "Expected 256 glyphs to be rendered, but found: " + glyphs.size() << std::endl;
			bump::die();
		}

		return blit_glyphs_as_tiles(glyphs, bump::font::blit_mode::MAX, tile_size, font.m_ft_font.get_line_height()).m_image;
	}

} // rog_ascii
	
int main(int argc, char** argv)
{
	if (argc != 5)
	{
		std::cerr << "Usage: rog_ascii_gen.exe input_font_file.ttf font_size_px, size_x_px size_y_px" << std::endl;
		return EXIT_FAILURE;
	}

	auto const in_file = std::string(argv[1]);
	auto const font_size = std::uint32_t(std::stoul(argv[2]));
	auto const tile_size = glm::i32vec2(std::stoul(argv[3]), std::stoul(argv[4]));
	auto const out_file = std::string("ascii_tiles.png"); // todo: get from args

	using namespace bump;

	auto ft_context = font::ft_context();
	auto ft_font = font::ft_font(ft_context.get_handle(), in_file);
	ft_font.set_pixel_size(font_size);
	auto hb_font = font::hb_font(ft_font.get_handle());

	auto font = font::font_asset{ std::move(ft_font), std::move(hb_font) };

	auto font_image = rog_ascii::render_ascii_tiles(ft_context, font, tile_size);

	write_png(out_file, font_image);

	std::clog << "done!" << std::endl;
}
