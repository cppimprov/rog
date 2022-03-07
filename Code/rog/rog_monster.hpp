#pragma once

#include "rog_direction.hpp"
#include "rog_random.hpp"

#include <entt.hpp>

namespace rog
{
	
	struct level;
	struct comp_position;

	entt::entity monster_create_entity(entt::registry& registry);
	void monster_move(level& level, entt::entity monster, comp_position& pos, random::rng_t& rng);

} // rog
