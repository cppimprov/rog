#pragma once

#include <entt.hpp>

#include <cstdint>

namespace rog
{
	
	struct c_actor;

	constexpr auto ACTOR_ENERGY_PER_CYCLE = std::int32_t{ 10 };
	constexpr auto ACTOR_ENERGY_PER_TURN = std::int32_t{ 100 };

	void actor_add_energy(c_actor& actor);
	bool actor_has_turn_energy(c_actor& actor);
	void actor_take_turn_energy(c_actor& actor);
	
} // rog
