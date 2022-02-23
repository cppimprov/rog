#include "rog_monster.hpp"

#include "rog_colors.hpp"
#include "rog_entity.hpp"
#include "rog_screen_cell.hpp"

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
	
} // rog
