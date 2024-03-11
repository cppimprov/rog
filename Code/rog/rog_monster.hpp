#pragma once

#include "rog_direction.hpp"
#include "rog_random.hpp"

#include <entt.hpp>

namespace rog
{
	
	struct level;
	struct c_position;

	entt::entity monster_create_entity(entt::registry& registry);
	void monster_move(level& level, entt::entity monster, c_position& pos, random::rng_t& rng);

} // rog
