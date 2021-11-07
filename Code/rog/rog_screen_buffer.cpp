#include "rog_screen_buffer.hpp"

#include <algorithm>

namespace rog
{
	
	void screen_buffer::resize(glm::size2 size, cell const& cell)
	{
		m_data.resize(size, cell);
	}

	void screen_buffer::clear()
	{
		fill_rect({ 0, 0 }, get_size(), { '\0', glm::vec3(0.f), glm::vec3(0.f) });
	}
	
	void screen_buffer::fill_rect(glm::size2 origin, glm::size2 size, cell const& cell)
	{
		auto const begin = glm::min(origin, get_size());
		auto const end = glm::min(origin + size, get_size());

		for (auto y : bump::range(begin.y, end.y))
			for (auto x : bump::range(begin.x, end.x))
				m_data.at({ x, y }) = cell;
	}

	void screen_buffer::print_char(glm::size2 origin, cell const& cell)
	{
		if (glm::any(glm::greaterThanEqual(origin, get_size())))
			return;
		
		m_data.at(origin) = cell;
	}

	void screen_buffer::print_string(glm::size2 origin, std::string const& s, glm::vec3 fg, glm::vec3 bg)
	{
		if (s.empty())
			return;

		if (glm::any(glm::greaterThanEqual(origin, get_size())))
			return;

		auto length = std::min(s.size(), get_size().x - origin.x);

		for (auto i = std::size_t{ 0 }; i != length; ++i)
			m_data.at({ origin.x + i, origin.y }) = { static_cast<std::uint8_t>(s[i]), fg, bg };
	}
	
} // rog
