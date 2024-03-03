#include "rog_screen.hpp"

namespace rog
{

	void screen::fill(cell const& cell)
	{
		fill_rect({ 0, 0 }, m_buffer.extents(), cell);
	}

	void screen::fill_rect(glm::size2 origin, glm::size2 size, cell const& cell)
	{
		auto const begin = glm::min(origin, m_buffer.extents());
		auto const end = glm::min(origin + size, m_buffer.extents());

		for (auto y : bump::range(begin.y, end.y))
			for (auto x : bump::range(begin.x, end.x))
				m_buffer.at({ x, y }) = cell;
	}

	void screen::resize(glm::size2 size, cell const& cell)
	{
		m_buffer.resize(size, cell);
	}

} // rog
