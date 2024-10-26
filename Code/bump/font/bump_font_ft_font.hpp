#pragma once

#include "bump_ptr_handle.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdint>
#include <string>

namespace bump
{
	
	namespace font
	{

		class ft_font : public ptr_handle<FT_FaceRec_>
		{
		public:

			ft_font() = default;
			explicit ft_font(FT_Library library, std::string const &filename, std::size_t face_index = 0);

			void set_pixel_size(std::uint32_t pixels_per_em);

			// values in FT's scaled 26.6 format (64ths of a pixel)
			std::int32_t get_ascent_266() const;
			std::int32_t get_descent_266() const; /* note: this value is negative! */
			std::int32_t get_line_height_266() const;
			std::int32_t get_underline_pos_266() const;
			std::int32_t get_underline_thickness_266() const;
			
			// values in pixels
			std::int32_t get_ascent_px() const;
			std::int32_t get_descent_px() const; /* note: this value is negative! */
			std::int32_t get_line_height_px() const;
			std::int32_t get_underline_pos_px() const;
			std::int32_t get_underline_thickness_px() const;
			
		};

	} // font
	
} // bump