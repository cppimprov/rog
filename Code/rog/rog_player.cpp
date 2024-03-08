#include "rog_player.hpp"

#include "rog_entity.hpp"
#include "rog_feature.hpp"
#include "rog_level.hpp"
#include "rog_screen.hpp"

#include <bump_log.hpp>

namespace rog
{
	
	entt::entity player_create_entity(entt::registry& registry)
	{
		auto player = registry.create();
		
		registry.emplace<comp_player_tag>(player);
		registry.emplace<comp_position>(player, glm::ivec2(0));
		registry.emplace<comp_visual>(player, screen_cell{ '@', colors::yellow, colors::black });
		registry.emplace<comp_actor>(player, 100); // start with enough energy to move

		return player;
	}

	bool player_can_use_stairs(level& level, stairs_direction dir)
	{
		auto const& grid = level.m_grid;
		auto const level_size = grid.extents();

		auto const& pos = level.m_registry.get<comp_position>(level.m_player);

		bump::die_if(level_size.x == 0 || level_size.y == 0);
		bump::die_if(pos.m_pos.x >= level_size.x || pos.m_pos.y >= level_size.y);

		if (dir == stairs_direction::UP)
		{
			if (!(grid.at(glm::size2(pos.m_pos)).m_flags & feature::flags::STAIRS_UP))
			{
				bump::log_info("There are no upward stairs here.");
				return false;
			}

			return true;
		}
		else
		{
			if (!(grid.at(glm::size2(pos.m_pos)).m_flags & feature::flags::STAIRS_DOWN))
			{
				bump::log_info("There are no downward stairs here.");
				return false;
			}

			return true;
		}
	}
	
} // rog
