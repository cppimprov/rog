#pragma once

#include <bump_grid.hpp>

#include <entt.hpp>

#include <cstdint>

namespace rog
{

	struct feature;

	struct level
	{
		std::int32_t m_depth;
		bump::grid2<feature> m_grid;

		entt::entity m_player;
		bump::grid2<entt::entity> m_actors;

		entt::registry m_registry;
	};

	bool is_walkable(level const& level, glm::size2 pos);
	bool is_occupied(level const& level, glm::size2 pos);

} // rog
