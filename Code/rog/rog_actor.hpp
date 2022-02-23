#pragma once

#include <entt.hpp>

#include <cstdint>

namespace rog
{
	
	constexpr auto ACTOR_ENERGY_PER_CYCLE = std::int32_t{ 10 };
	constexpr auto ACTOR_ENERGY_PER_TURN = std::int32_t{ 100 };

	void actors_add_energy(entt::registry& registry);
	bool actor_has_turn_energy(entt::handle actor_handle);
	void actor_take_turn_energy(entt::handle actor_handle);
	
} // rog
