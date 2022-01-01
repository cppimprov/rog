#pragma once

#include "bump_ptr_handle.hpp"

#include <hb.h>

#include <span>
#include <string>

namespace bump
{

	namespace font
	{

		class hb_shaper : public ptr_handle<hb_buffer_t>
		{
		public:
			
			explicit hb_shaper(hb_direction_t direction, hb_script_t script, hb_language_t language);

			void set_direction(hb_direction_t direction);
			hb_direction_t get_direction() const;

			void set_script(hb_script_t script);
			hb_script_t get_script() const;

			void set_language(hb_language_t language);
			hb_language_t get_language() const;

			void shape(hb_font_t *harfbuzz_font, std::string const &utf8_str, std::span<hb_feature_t> features = {});

			std::span<hb_glyph_info_t> get_glyph_info() const;
			std::span<hb_glyph_position_t> get_glyph_positions() const;
		};

	} // font

} // bump
