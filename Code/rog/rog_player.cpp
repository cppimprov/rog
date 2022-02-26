#include "rog_player.hpp"

#include "rog_entity.hpp"
#include "rog_feature.hpp"
#include "rog_level.hpp"
#include "rog_screen_cell.hpp"

#include <bump_log.hpp>

namespace rog
{
	
	entt::entity player_create_entity(entt::registry& registry)
	{
		auto player = registry.create();
		
		registry.emplace<comp_player_tag>(player);
		registry.emplace<comp_position>(player, glm::size2(0));
		registry.emplace<comp_visual>(player, screen::cell{ '@', colors::yellow, colors::black });
		registry.emplace<comp_actor>(player, 100); // start with enough energy to move

		return player;
	}

	bool player_move(entt::handle player_handle, level& level, direction dir)
	{
		auto const vec = get_direction_vector(dir);
		auto const& grid = level.m_grid;
		auto const level_size = grid.extents();

		auto& pos = player_handle.get<comp_position>();

		bump::die_if(level_size.x == 0 || level_size.y == 0);
		bump::die_if(pos.m_pos.x >= level_size.x || pos.m_pos.y >= level_size.y);

		if (pos.m_pos.x == 0 && vec.x < 0) return false;
		if (pos.m_pos.x == level_size.x - 1 && vec.x > 0) return false;
		if (pos.m_pos.y == 0 && vec.y < 0) return false;
		if (pos.m_pos.y == level_size.y - 1 && vec.y > 0) return false;

		auto const target = pos.m_pos + glm::size2(vec);

		if (!is_walkable(level, target) || is_occupied(level, target))
			return false;
		
		level.m_actors.at(pos.m_pos) = entt::null;

		pos.m_pos = target;
		
		level.m_actors.at(pos.m_pos) = player_handle.entity();

		return true;
	}

	bool player_can_use_stairs(entt::handle player_handle, level const& level, stairs_direction dir)
	{
		auto const& grid = level.m_grid;
		auto const level_size = grid.extents();

		auto const& pos = player_handle.get<comp_position>();

		bump::die_if(level_size.x == 0 || level_size.y == 0);
		bump::die_if(pos.m_pos.x >= level_size.x || pos.m_pos.y >= level_size.y);

		if (dir == stairs_direction::UP)
		{
			if (!(grid.at(pos.m_pos).m_flags & feature::flags::STAIRS_UP))
			{
				bump::log_info("There are no upward stairs here.");
				return false;
			}

			return true;
		}
		else
		{
			if (!(grid.at(pos.m_pos).m_flags & feature::flags::STAIRS_DOWN))
			{
				bump::log_info("There are no downward stairs here.");
				return false;
			}

			return true;
		}
	}
	
} // rog
