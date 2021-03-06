#pragma once

#include "rog_direction.hpp"
#include "rog_feature.hpp"
#include "rog_screen_cell.hpp"

#include <bump_grid.hpp>
#include <bump_math.hpp>

#include <entt.hpp>

namespace rog
{

	struct level;
	
	entt::entity player_create_entity(entt::registry& registry);
	bool player_can_use_stairs(level& level, stairs_direction dir);

} // rog
