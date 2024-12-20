#pragma once

#include "bump_font_asset.hpp"
#include "bump_font_ft_context.hpp"
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

	text_texture render_text_to_gl_texture(font::ft_context const& ft_context, font::font_asset const& font, std::string const& utf8_text);
	text_texture render_text_outline_to_gl_texture(font::ft_context const& ft_context, font::font_asset const& font, std::string const& utf8_text, double outline_width);

	std::int32_t measure_text(font::ft_context const& ft_context, font::font_asset const& font, std::string const& utf8_text);
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

	charmap_texture render_charmap(font::ft_context const& ft_context, font::font_asset const& font, std::string const& chars);
	
	gl::texture_2d_array render_ascii_tiles(font::ft_context const& ft_context, font::font_asset const& font, glm::i32vec2 tile_size);

} // bump
