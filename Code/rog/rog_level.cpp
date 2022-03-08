#include "rog_level.hpp"

#include "rog_direction.hpp"
#include "rog_entity.hpp"
#include "rog_feature.hpp"

namespace rog
{
	
	bool is_walkable(level const& level, glm::size2 pos)
	{
		return !(level.m_grid.at(pos).m_flags & feature::flags::NO_WALK);
	}

	bool is_occupied(level const& level, glm::size2 pos)
	{
		return level.m_actors.at(pos) != entt::null;
	}

	bool move_actor(level& level, entt::entity entity, comp_position& pos, direction dir)
	{
		auto const vec = get_direction_vector(dir);
		auto const& grid = level.m_grid;
		auto const level_size = grid.extents();

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
		level.m_actors.at(pos.m_pos) = entity;

		return true;
	}

} // rog
