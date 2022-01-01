#pragma once

#include "rog_screen_cell.hpp"

#include <bump_math.hpp>

namespace rog
{
	
	struct player
	{
		glm::size2 m_position;
		screen::cell m_cell;
	};

} // rog