#include "bump_font_hb_shaper.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"
#include "bump_narrow_cast.hpp"

#include <algorithm>

namespace bump
{
	
	namespace font
	{

		hb_shaper::hb_shaper()
		{
			auto handle = hb_buffer_create();

			if (!handle)
			{
				log_error("hb_buffer_create() failed");
				die();
			}

			reset(handle, [] (hb_buffer_t* b) { hb_buffer_destroy(b); });
		}
		
		hb_shaper::hb_shaper(hb_direction_t direction, hb_script_t script, hb_language_t language):
			hb_shaper()
		{
			set_direction(direction);
			set_script(script);
			set_language(language);
		}
		
		void hb_shaper::set_direction(hb_direction_t direction)
		{
			die_if(!is_valid());
			hb_buffer_set_direction(get_handle(), direction);
		}

		hb_direction_t hb_shaper::get_direction() const
		{
			die_if(!is_valid());
			return hb_buffer_get_direction(get_handle());
		}

		void hb_shaper::set_script(hb_script_t script)
		{
			die_if(!is_valid());
			hb_buffer_set_script(get_handle(), script);
		}

		hb_script_t hb_shaper::get_script() const
		{
			die_if(!is_valid());
			return hb_buffer_get_script(get_handle());
		}
		
		void hb_shaper::set_language(hb_language_t language)
		{
			die_if(!is_valid());
			hb_buffer_set_language(get_handle(), language);
		}
		
		hb_language_t hb_shaper::get_language() const
		{
			die_if(!is_valid());
			return hb_buffer_get_language(get_handle());
		}

		void hb_shaper::add_utf8(std::string_view utf8_str)
		{
			die_if(!is_valid());
			hb_buffer_add_utf8(get_handle(), utf8_str.data(), narrow_cast<int>(utf8_str.size()), 0, -1);
		}

		void hb_shaper::clear_contents()
		{
			die_if(!is_valid());
			hb_buffer_clear_contents(get_handle());
		}

		void hb_shaper::clear()
		{
			die_if(!is_valid());
			hb_buffer_reset(get_handle());
		}

		void hb_shaper::shape(hb_font_t* harfbuzz_font, std::span<hb_feature_t> features)
		{
			die_if(!is_valid());
			hb_shape(harfbuzz_font, get_handle(), features.data(), narrow_cast<unsigned int>(features.size()));
		}

		std::uint32_t hb_shaper::next_cluster(std::uint32_t start) const
		{
			die_if(!is_valid());

			auto const info = get_glyph_info();
			auto const next = std::upper_bound(info.begin(), info.end(),
				start, [] (std::uint32_t value, hb_glyph_info_t const& info) { return value < info.cluster; });

			if (next == info.end())
				return std::uint32_t(-1); // ugh
			
			return next->cluster;
		}

		std::uint32_t hb_shaper::prev_cluster(std::uint32_t start) const
		{
			die_if(!is_valid());

			auto const info = get_glyph_info();
			auto const next = std::upper_bound(info.rbegin(), info.rend(),
				start, [] (std::uint32_t value, hb_glyph_info_t const& info) { return value > info.cluster; });

			if (next == info.rend())
				return 0;
			
			return next->cluster;
		}

		std::span<hb_glyph_info_t> hb_shaper::get_glyph_info() const
		{
			die_if(!is_valid());

			auto const l = hb_buffer_get_length(get_handle());
			(void)l;

			auto length = 0u;
			auto data = hb_buffer_get_glyph_infos(get_handle(), &length);

			return { data, length };
		}

		std::span<hb_glyph_position_t> hb_shaper::get_glyph_positions() const
		{
			die_if(!is_valid());

			auto length = 0u;
			auto data = hb_buffer_get_glyph_positions(get_handle(), &length);

			return { data, length };
		}

		std::string hb_shaper::serialize(hb_font_t *hb_font) const
		{
			auto const items = hb_buffer_get_length(get_handle());

			auto out = std::string();

			for (auto i = unsigned int{ 0 }; i != items; )
			{
				auto const str_size = 4096;
				auto str = std::string(str_size, '\0');
				auto bytes = unsigned int{ 0 };

				auto const n = hb_buffer_serialize_glyphs(get_handle(),
					i, items,
					str.data(), str_size, &bytes,
					hb_font,
					HB_BUFFER_SERIALIZE_FORMAT_JSON,
					HB_BUFFER_SERIALIZE_FLAG_DEFAULT);

				str.resize(bytes);
				out += str;
				i += n;
			}

			return out;
		}
		
	} // font
	
} // bump
