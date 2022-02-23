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
		bump::grid2<entt::entity> m_actors;

		entt::registry m_registry;
	};

} // rog
