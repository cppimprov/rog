#pragma once

#include "rog_screen_cell.hpp"

#include <bump_grid.hpp>
#include <bump_math.hpp>

#include <string>

namespace rog
{

	struct feature;
	struct player;
	
	namespace screen
	{
		
		void fill(bump::grid2<cell>& screen, cell const& cell);
		void fill_rect(bump::grid2<cell>& screen, glm::size2 origin, glm::size2 size, cell const& cell);

		void resize(bump::grid2<cell>& screen, glm::ivec2 window_size, glm::ivec2 tile_size, cell const& cell);
		
		glm::size2 get_panel_origin(glm::size2 level_size, glm::size2 panel_size, glm::size2 focus);

		void draw(bump::grid2<cell>& buffer, bump::grid2<feature> const& grid, glm::size2 player_pos, cell const& cell);

	} // screen

} // rog
