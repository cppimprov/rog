#include "rog_monster.hpp"

#include "rog_colors.hpp"
#include "rog_ecs.hpp"
#include "rog_level.hpp"
#include "rog_screen.hpp"

namespace rog
{
	
	entt::entity monster_create_entity(entt::registry& registry)
	{
		auto monster = registry.create();

		registry.emplace<c_monster_tag>(monster);
		registry.emplace<c_position>(monster, glm::ivec2(0));
		registry.emplace<c_visual>(monster, screen_cell{ 'T', colors::green, colors::black });
		registry.emplace<c_actor>(monster, 0); // start without energy to move (player has advantage here)
		
		return monster;
	}
	
	void monster_move(level& level, entt::entity monster, c_position& pos, random::rng_t& rng)
	{
		// todo: random direction function? (and ignore 5?)
		auto const dir_idx = random::rand_range(rng, 0, 8);
		auto const dir = static_cast<direction>(dir_idx);

		(void)move_actor(level, monster, pos, dir); // todo: try a different direction?
	}

} // rog
