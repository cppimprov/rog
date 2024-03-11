#pragma once

#include "rog_screen.hpp"

#include <bump_math.hpp>

namespace rog
{

	struct c_position { glm::ivec2 m_pos; };
	struct c_visual { screen_cell m_cell; };
	struct c_actor { std::int32_t m_energy = 0; };

	struct c_player_tag { };
	struct c_monster_tag { };
	struct c_object_tag { };

} // rog
