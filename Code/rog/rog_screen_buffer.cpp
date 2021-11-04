#include "rog_screen_buffer.hpp"

#include <algorithm>

namespace rog
{
	
	void screen_buffer::clear()
	{
		fill_rect({ 0, 0 }, get_size(), '\0');
	}
	
	void screen_buffer::fill_rect(glm::size2 origin, glm::size2 size, char value)
	{
		auto const begin = glm::min(origin, get_size());
		auto const end = glm::min(origin + size, get_size());

		for (auto y : bump::range(begin.y, end.y))
			for (auto x : bump::range(begin.x, end.x))
				m_data.at({ x, y }) = value;
	}

	void screen_buffer::print_char(glm::size2 origin, char value)
	{
		if (glm::any(glm::greaterThanEqual(origin, get_size())))
			return;
		
		m_data.at(origin) = value;
	}

	void screen_buffer::print_string(glm::size2 origin, std::string const& s)
	{
		if (s.empty())
			return;

		if (glm::any(glm::greaterThanEqual(origin, get_size())))
			return;

		auto length = std::min(s.size(), get_size().x - origin.x);

		for (auto i = std::size_t{ 0 }; i != length; ++i)
			m_data.at({ origin.x + i, origin.y }) = s[i];
	}
	
} // rog
