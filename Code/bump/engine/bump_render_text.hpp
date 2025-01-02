#pragma once

#include "bump_font.hpp"
#include "bump_gl_texture.hpp"

#include <array>
#include <map>
#include <string>

namespace bump
{
	
	struct text_texture
	{
		glm::i32vec2 m_pos;
		glm::i32vec2 m_advance;
		gl::texture_2d m_texture;
	};

	text_texture render_text_to_gl_texture(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, font::hb_shaper const& hb_shaper);
	text_texture render_text_to_gl_texture(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, std::string_view utf8_text);
	text_texture render_text_outline_to_gl_texture(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, std::string_view utf8_text, double outline_width);

	std::int32_t measure_text(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, font::hb_shaper const& hb_shaper, std::size_t start, std::size_t end);
	std::int32_t measure_text(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, font::hb_shaper const& hb_shaper);
	std::int32_t measure_text(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, std::string_view utf8_text);
	// todo: outline version?

	struct charmap_texture
	{
		struct glyph_info
		{
			glm::ivec2 m_origin; // origin in texture (pixel coords)
			glm::ivec2 m_size;   // pixel size
		};

		glm::ivec2 m_pos;
		std::map<char, glyph_info> m_char_positions;
		gl::texture_2d m_texture;
	};

	charmap_texture render_charmap(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, std::string const& chars);

} // bump
