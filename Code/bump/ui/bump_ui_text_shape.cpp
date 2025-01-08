#include "bump_ui_text_shape.hpp"

#include "bump_narrow_cast.hpp"

// eww...
#pragma warning(push)
#pragma warning(disable: 4244 4267 4100 4458)
#include <hb-utf.hh>
#undef min
#undef max
#undef DELETE
#pragma warning(pop)

#include <algorithm>

namespace bump::ui
{
	
	text_shape::text_shape(font::ft_context const& ft_context, font::ft_font const& ft_font, font::hb_font const& hb_font, std::string text = ""):
		m_ft_context(&ft_context),
		m_ft_font(&ft_font),
		m_hb_font(&hb_font),
		m_shaper(),
		m_text(text),
		m_max_length(255)
	{
		reshape();
	}

	void text_shape::set(std::string_view text)
	{
		m_text = text;

		if (m_text.size() > m_max_length)
			m_text.resize(m_max_length);

		reshape();
	}

	std::size_t text_shape::insert(std::size_t pos, std::string_view text)
	{
		// calculate available space
		auto const space = m_max_length - m_text.size();
		auto const len = std::min(space, text.size());

		// insert as much text as possible
		m_text.insert(pos, std::string_view(text.data(), len));

		reshape();

		return pos + len;
	}

	void text_shape::erase(std::size_t pos, std::size_t length)
	{
		m_text.erase(pos, length);

		reshape();
	}

	std::size_t text_shape::size() const
	{
		return m_text.size();
	}

	void text_shape::set_max_length(std::size_t length)
	{
		m_max_length = length;

		if (m_text.size() > m_max_length)
			m_text.resize(m_max_length);
		
		reshape();
	}

	std::size_t text_shape::next_word(std::size_t pos, std::ptrdiff_t diff) const
	{
		if (m_text.empty())
			return 0;

		auto const begin = m_text.begin() + pos;
		auto const end = m_text.end();
		auto const rbegin = std::reverse_iterator(begin);
		auto const rend = m_text.rend();

		auto constexpr is_space = [] (char c) {
			// todo: unicode has other types of space?
			auto const space = { ' ', '\f', '\n', '\r', '\t', '\v' };
			return std::find(space.begin(), space.end(), c) != space.end();
		};

		auto const last_adj_whitespace = diff < 0 ?
			std::find_if_not(rbegin, rend, is_space).base() :
			std::find_if_not(begin, end, is_space);

		auto const first_space = diff < 0 ?
			std::find_if(std::reverse_iterator(last_adj_whitespace), rend, is_space).base() :
			std::find_if(last_adj_whitespace, end, is_space);
		
		return static_cast<std::size_t>(first_space - m_text.begin());
	}

	std::size_t text_shape::next_cluster(std::size_t pos, std::ptrdiff_t diff) const
	{
		auto start = narrow_cast<std::uint32_t>(pos);
		auto const n = narrow_cast<std::size_t>(std::abs(diff));

		for (auto i = std::size_t{ 0 }; i != n; ++i)
			start = (diff > 0) ? m_shaper.next_cluster(start) : m_shaper.prev_cluster(start);
		
		if (start == std::uint32_t(-1)) // ugh
			start = narrow_cast<std::uint32_t>(m_text.size());

		return std::size_t{ start };
	}

	std::size_t text_shape::next_codepoint(std::size_t pos, std::ptrdiff_t diff) const
	{
		// eww...
		auto const beg = reinterpret_cast<hb_utf8_t::codepoint_t const*>(m_text.data());
		auto const end = reinterpret_cast<hb_utf8_t::codepoint_t const*>(m_text.data() + m_text.size());
		auto start = reinterpret_cast<hb_utf8_t::codepoint_t const*>(m_text.data() + pos);
		auto const n = narrow_cast<std::size_t>(std::abs(diff));

		for (auto i = std::size_t{ 0 }; i != n; ++i)
		{
			auto u = hb_codepoint_t{ 0 };
			start = (diff > 0) ? hb_utf8_t::next(start, end, &u, 0) : hb_utf8_t::prev(start, beg, &u, 0);
			(void)u;
		}

		return static_cast<std::size_t>(reinterpret_cast<char const*>(start) - m_text.data());
	};

	void text_shape::reshape()
	{
		m_shaper.clear_contents();
		m_shaper.set_direction(HB_DIRECTION_LTR);
		m_shaper.set_script(HB_SCRIPT_LATIN); // todo: don't hard-code this...
		m_shaper.set_language(hb_language_from_string("en", -1));
		m_shaper.add_utf8(m_text);
		m_shaper.shape(m_hb_font->get_handle());
	}

	text_texture text_shape::render()
	{
		return render_text_to_gl_texture(*m_ft_context, *m_ft_font, *m_hb_font, m_shaper);
	}

	std::int32_t text_shape::measure(std::size_t start, std::size_t end)
	{
		die_if(start > end);
		return measure_text(*m_ft_context, *m_ft_font, *m_hb_font, m_shaper, start, end);
	}

} // bump::ui
