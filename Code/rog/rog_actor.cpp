#include "rog_actor.hpp"

#include "rog_ecs.hpp"

namespace rog
{
	
	void actor_add_energy(c_actor& actor)
	{
		actor.m_energy += ACTOR_ENERGY_PER_CYCLE;
	}

	bool actor_has_turn_energy(c_actor& actor)
	{
		return (actor.m_energy >= ACTOR_ENERGY_PER_TURN);
	}

	void actor_take_turn_energy(c_actor& actor)
	{
		actor.m_energy -= ACTOR_ENERGY_PER_TURN;
	}
	
} // rog
