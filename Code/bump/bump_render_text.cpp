#include "bump_render_text.hpp"

#include "bump_font.hpp"
#include "bump_gl_error.hpp"
#include "bump_load_image.hpp"
#include "bump_narrow_cast.hpp"
#include "bump_range.hpp"

#include <GL/glew.h>

#include <numeric>

namespace bump
{

	namespace
	{

		gl::texture_2d text_image_to_gl_texture(image<std::uint8_t> const& image)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // :(

			auto texture = gl::texture_2d();
			texture.set_data(narrow_cast<glm::i32vec2>(image.size()), GL_R8, gl::make_texture_data_source(GL_RED, image.data()));

			gl::die_if_error();
			return texture;
		}

		gl::texture_2d_array text_image_to_gl_texture_array(image<std::uint8_t> const& image, std::size_t num_layers)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // :(

			auto texture = gl::texture_2d_array();
			auto texture_size_sz = glm::size3{ image.size().x, image.size().y / num_layers, num_layers };
			texture.set_data(narrow_cast<glm::i32vec3>(texture_size_sz), GL_R8, gl::make_texture_data_source(GL_RED, image.data()));

			gl::die_if_error();
			return texture;
		}

	} // unnamed
	
	text_texture render_text_to_gl_texture(font::ft_context const& ft_context, font::font_asset const& font, std::string const& utf8_text)
	{
		auto hb_shaper = font::hb_shaper(HB_DIRECTION_LTR, HB_SCRIPT_LATIN, hb_language_from_string("en", -1));
		hb_shaper.shape(font.m_hb_font.get_handle(), utf8_text);

		auto glyphs = render_glyphs(ft_context, font.m_ft_font, font.m_hb_font, hb_shaper);
		auto image = blit_glyphs(glyphs, font::blit_mode::MAX);
		
		return { image.m_pos, text_image_to_gl_texture(image.m_image) };
	}
	
	text_texture render_text_outline_to_gl_texture(font::ft_context const& ft_context, font::font_asset const& font, std::string const& utf8_text, double outline_width)
	{
		auto hb_shaper = font::hb_shaper(HB_DIRECTION_LTR, HB_SCRIPT_LATIN, hb_language_from_string("en", -1));
		hb_shaper.shape(font.m_hb_font.get_handle(), utf8_text);

		auto glyphs = render_glyphs(ft_context, font.m_ft_font, font.m_hb_font, hb_shaper, { outline_width });
		auto image = blit_glyphs(glyphs, font::blit_mode::MAX);
		
		return { image.m_pos, text_image_to_gl_texture(image.m_image) };
	}

	charmap_texture render_charmap(font::ft_context const& ft_context, font::font_asset const& font, std::string const& chars)
	{
		auto hb_shaper = font::hb_shaper(HB_DIRECTION_LTR, HB_SCRIPT_LATIN, hb_language_from_string("en", -1));
		hb_shaper.shape(font.m_hb_font.get_handle(), chars);

		auto glyphs = render_glyphs(ft_context, font.m_ft_font, font.m_hb_font, hb_shaper);
		auto image = blit_glyphs(glyphs, font::blit_mode::MAX);
		auto texture = text_image_to_gl_texture(image.m_image);

		die_if(glyphs.size() != chars.size());

		auto map = std::map<char, charmap_texture::glyph_info>();

		for (auto i = std::size_t{ 0 }; i != glyphs.size(); ++i)
		{
			// make glyph origins relative to texture origin
			auto const origin = glyphs[i].m_pos - image.m_pos;
			auto const size = glm::ivec2(glyphs[i].m_image.size());
			auto const inserted = map.insert({ chars[i], charmap_texture::glyph_info{ origin, size } }).second;
			die_if(!inserted); // chars in char_text must be unique!
		}
		
		return { image.m_pos, std::move(map), std::move(texture) };
	}

	gl::texture_2d_array render_ascii_tiles(font::ft_context const& ft_context, font::font_asset const& font, glm::i32vec2 tile_size)
	{
		using namespace std::string_literals;

		// 256 characters encoded as utf-8.
		// control characters (0 to 31 and some chars >127) are encoded as nulls.
		// characters from 128 to 255 are extended ascii (CP 1251) converted to utf-8.
		auto const chars_utf8 = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\0€\0‚ƒ„…†‡ˆ‰Š‹Œ\0Ž\0\0‘’“”•–—˜™š›œ\0žŸ ¡¢£¤¥¦§¨©ª«¬\0®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"s;

		auto hb_shaper = font::hb_shaper(HB_DIRECTION_LTR, HB_SCRIPT_LATIN, hb_language_from_string("en", -1));
		hb_shaper.shape(font.m_hb_font.get_handle(), chars_utf8);

		auto glyphs = render_glyphs(ft_context, font.m_ft_font, font.m_hb_font, hb_shaper);
		die_if(glyphs.size() != 256);

		auto image = blit_glyphs_as_tiles(glyphs, font::blit_mode::MAX, tile_size, font.m_ft_font.get_line_height());
		auto texture = text_image_to_gl_texture_array(image.m_image, glyphs.size());

		return texture;
	}
	
} // bump
