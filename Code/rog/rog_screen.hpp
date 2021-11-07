#pragma once

#include <bump_grid.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/std_based_type.hpp>

#include <string>

namespace rog
{
	
	namespace screen
	{
		
		struct cell
		{
			std::uint8_t m_value;
			glm::vec3 m_fg;
			glm::vec3 m_bg;
		};

		using buffer = bump::grid2<cell>;

		void fill(buffer& screen, cell const& cell);
		void fill_rect(buffer& screen, glm::size2 origin, glm::size2 size, cell const& cell);
		void print_char(buffer& screen, glm::size2 origin, cell const& cell);
		void print_string(buffer& screen, glm::size2 origin, std::string const& s, glm::vec3 fg, glm::vec3 bg);
		void resize(buffer& screen, glm::ivec2 window_size, glm::ivec2 tile_size, cell const& cell);
		
	} // screen

} // rog
