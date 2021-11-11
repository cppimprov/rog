#pragma once

#include "rog_screen_cell.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/std_based_type.hpp>

namespace rog
{
	
	struct player
	{
		glm::size2 m_position;
		screen::cell m_cell;
	};

} // rog