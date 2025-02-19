#include "bump_render_text.hpp"

#include "bump_font.hpp"
#include "bump_gl_error.hpp"
#include "bump_load_image.hpp"
#include "bump_narrow_cast.hpp"
#include "bump_range.hpp"

#include <GL/glew.h>

#include <iostream>
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
			auto const texture_size_sz = glm::size3{ image.size().x, image.size().y / num_layers, num_layers };
			texture.set_data(narrow_cast<glm::i32vec3>(texture_size_sz), GL_R8, gl::make_texture_data_source(GL_RED, image.data()));

			gl::die_if_error();
			return texture;
		}

	} // unnamed

	font::hb_shaper make_shaper(font::hb_font const& hb_font, std::string_view utf8_text)
	{
		auto hb_shaper = font::hb_shaper(HB_DIRECTION_LTR, HB_SCRIPT_LATIN, hb_language_from_string("en", -1));
		hb_shaper.add_utf8(utf8_text);
		hb_shaper.shape(hb_font.get_handle());

		return hb_shaper;
	}

	text_texture render_text_to_gl_texture(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, font::hb_shaper const& hb_shaper)
	{
		auto const glyphs = render_glyphs(ft_context, ft_font, hb_font, hb_shaper);
		auto const image = blit_glyphs(glyphs, font::blit_mode::MAX);

		return { image.m_pos, image.m_advance, text_image_to_gl_texture(image.m_image) };
	}
	
	text_texture render_text_to_gl_texture(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, std::string_view utf8_text)
	{
		return render_text_to_gl_texture(ft_context, ft_font, hb_font, make_shaper(hb_font, utf8_text));
	}
	
	text_texture render_text_outline_to_gl_texture(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, std::string_view utf8_text, double outline_width)
	{
		auto hb_shaper = make_shaper(hb_font, utf8_text);
		auto const glyphs = render_glyphs(ft_context, ft_font, hb_font, hb_shaper, { outline_width });
		auto const image = blit_glyphs(glyphs, font::blit_mode::MAX);
		
		return { image.m_pos, image.m_advance, text_image_to_gl_texture(image.m_image) };
	}

	std::int32_t measure_text(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, font::hb_shaper const& hb_shaper, std::size_t start, std::size_t end)
	{
		return measure_glyphs(ft_context, ft_font, hb_font, hb_shaper, start, end);
	}

	std::int32_t measure_text(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, font::hb_shaper const& hb_shaper)
	{
		return measure_glyphs(ft_context, ft_font, hb_font, hb_shaper);
	}

	std::int32_t measure_text(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, std::string_view utf8_text)
	{
		return measure_text(ft_context, ft_font, hb_font, make_shaper(hb_font, utf8_text));
	}

	charmap_texture render_charmap(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, std::string const& chars)
	{
		auto const hb_shaper = make_shaper(hb_font, chars);
		auto const glyphs = render_glyphs(ft_context, ft_font, hb_font, hb_shaper);
		auto const image = blit_glyphs(glyphs, font::blit_mode::MAX);
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

} // bump
