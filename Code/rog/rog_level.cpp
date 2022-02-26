#include "rog_level.hpp"

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
	
} // rog
