#pragma once

#include "bump_image.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace bump
{
	
	namespace font
	{

		class ft_context;
		class ft_font;
		class hb_font;
		class hb_shaper;
		
		struct glyph_image
		{
			glm::i32vec2 m_pos = { 0, 0 }; // bottom left
			glm::i32vec2 m_advance = { 0, 0 }; // advance for this glyph
			image<std::uint8_t> m_image; // note: the first pixel in the vector is the bottom left
		};
		
		std::vector<glyph_image> render_glyphs(ft_context const& ft_context, ft_font const& ft_font, hb_font const& hb_font, hb_shaper const& hb_shaper, std::optional<double> stroke_width = { });
		std::int32_t measure_glyphs(ft_context const& ft_context, ft_font const& ft_font, hb_font const& hb_font, hb_shaper const& hb_shaper);
		std::int32_t measure_glyphs(ft_context const& ft_context, ft_font const& ft_font, hb_font const& hb_font, hb_shaper const& hb_shaper, std::size_t start, std::size_t end);

		enum class blit_mode { ADD, MAX };
		void blit_image(image<std::uint8_t>& dst, glm::size2 dst_pos, image<std::uint8_t> const& src, blit_mode mode);
		glyph_image blit_glyphs(std::vector<glyph_image> const& glyphs, blit_mode mode);

	} // font
	
} // bump
