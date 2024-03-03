#pragma once

#include <bump_grid.hpp>
#include <bump_math.hpp>

#include <cstdint>

namespace rog
{


	struct screen
	{
		
		struct cell
		{
			std::uint8_t m_value;
			glm::vec3 m_fg;
			glm::vec3 m_bg;
		};

		void fill(cell const& cell);
		void fill_rect(glm::size2 origin, glm::size2 size, cell const& cell);

		void resize(glm::size2 size, cell const& cell);

		bump::grid2<cell> m_buffer;
	};

} // rog
