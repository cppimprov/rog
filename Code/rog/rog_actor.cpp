#include "rog_actor.hpp"

#include "rog_entity.hpp"

namespace rog
{
	
	void actors_add_energy(entt::registry& registry)
	{
		auto view = registry.view<comp_actor>();

		for (auto& actor : view)
			view.get<comp_actor>(actor).m_energy += ACTOR_ENERGY_PER_CYCLE;
	}

	bool actor_has_turn_energy(entt::handle actor_handle)
	{
		return actor_handle.get<comp_actor>().m_energy >= ACTOR_ENERGY_PER_TURN;
	}

	void actor_take_turn_energy(entt::handle actor_handle)
	{
		actor_handle.get<comp_actor>().m_energy -= ACTOR_ENERGY_PER_TURN;
	}
	
} // rog
