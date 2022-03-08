#pragma once

#include "rog_direction.hpp"

#include <bump_grid.hpp>

#include <entt.hpp>

#include <cstdint>

namespace rog
{

	struct comp_position;
	struct feature;

	struct level
	{
		std::int32_t m_depth;
		bump::grid2<feature> m_grid;

		entt::registry m_registry;
		entt::entity m_player;
		bump::grid2<entt::entity> m_actors;
	};

	bool is_walkable(level const& level, glm::size2 pos);
	bool is_occupied(level const& level, glm::size2 pos);

	bool move_actor(level& level, entt::entity entity, comp_position& pos, glm::size2 target);
	bool move_actor(level& level, entt::entity entity, comp_position& pos, direction dir);

	std::vector<glm::size2> find_path(bump::grid2<feature> const& grid, glm::size2 src, glm::size2 dst);

} // rog
