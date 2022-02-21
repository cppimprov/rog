#pragma once

#include "rog_screen_cell.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/std_based_type.hpp>

namespace rog
{
	
	struct comp_position { glm::size2 m_pos; };
	struct comp_visual { screen::cell m_cell; };
	struct comp_actor { std::int32_t m_energy = 0; };

	struct comp_player_tag { };
	struct comp_monster_tag { };
	struct comp_object_tag { };
		
} // rog
