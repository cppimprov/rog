#include "rog_monster.hpp"

#include "rog_colors.hpp"
#include "rog_entity.hpp"
#include "rog_level.hpp"
#include "rog_screen.hpp"

namespace rog
{
	
	entt::entity monster_create_entity(entt::registry& registry)
	{
		auto monster = registry.create();

		registry.emplace<comp_monster_tag>(monster);
		registry.emplace<comp_position>(monster, glm::size2(0));
		registry.emplace<comp_visual>(monster, screen::cell{ 'T', colors::green, colors::black });
		registry.emplace<comp_actor>(monster, 0); // start without energy to move (player has advantage here)
		
		return monster;
	}
	
	void monster_move(level& level, entt::entity monster, comp_position& pos, random::rng_t& rng)
	{
		// todo: random direction function? (and ignore 5?)
		auto const dir_idx = random::rand_range(rng, 0, 8);
		auto const dir = static_cast<direction>(dir_idx);

		(void)move_actor(level, monster, pos, dir); // todo: try a different direction?
	}

} // rog
