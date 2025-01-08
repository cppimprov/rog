#pragma once

#include "bump_font.hpp"
#include "bump_render_text.hpp"

#include <cstdint>
#include <string>

namespace bump::ui
{

	struct text_shape
	{
	public:

		explicit text_shape(
			font::ft_context const& ft_context,
			font::ft_font const& ft_font,
			font::hb_font const& hb_font,
			std::string text);

		void set(std::string_view text);
		std::string const& get() const { return m_text; }

		void set_font(font::ft_font const& ft_font, font::hb_font const& hb_font) { m_ft_font = &ft_font; m_hb_font = &hb_font; reshape(); }
		font::ft_font const& get_ft_font() const { return *m_ft_font; }
		font::hb_font const& get_hb_font() const { return *m_hb_font; }

		std::size_t insert(std::size_t pos, std::string_view text);
		void erase(std::size_t pos, std::size_t length);

		std::size_t size() const;

		void set_max_length(std::size_t length);
		std::size_t get_max_length() const { return m_max_length; }

		std::size_t next_word(std::size_t pos, std::ptrdiff_t diff) const;
		std::size_t next_cluster(std::size_t pos, std::ptrdiff_t diff) const;
		std::size_t next_codepoint(std::size_t pos, std::ptrdiff_t diff) const;

		void reshape();

		text_texture render();
		std::int32_t measure(std::size_t start, std::size_t end);

	private:

		font::ft_context const* m_ft_context;
		font::ft_font const* m_ft_font;
		font::hb_font const* m_hb_font;
		font::hb_shaper m_shaper;

		std::string m_text;
		std::size_t m_max_length;
	};

} // bump::ui
