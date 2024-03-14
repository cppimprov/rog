#pragma once

#include "rog_direction.hpp"
#include "rog_feature.hpp"

#include <bump_aabb.hpp>
#include <bump_grid.hpp>

#include <entt.hpp>

#include <cstdint>

namespace rog
{

	struct c_position;
	struct screen_buffer;

	struct level
	{
		glm::ivec2 size() const { return m_grid.extents(); }
		bool in_bounds(glm::ivec2 pos) const { return bump::iaabb2{ glm::ivec2(0), size() }.contains(pos); }

		bool is_walkable(glm::ivec2 pos) const;
		bool is_occupied(glm::ivec2 pos) const;

		bool move_actor(entt::entity entity, c_position& pos, glm::ivec2 target);
		bool move_actor(entt::entity entity, c_position& pos, direction dir);

		bump::iaabb2 get_map_panel(glm::ivec2 panel_size, glm::ivec2 focus_lv) const;
		bump::iaabb2 get_map_panel(glm::ivec2 panel_size) const;

		std::int32_t m_depth;
		bump::grid2<feature, glm::ivec2> m_grid;

		entt::registry m_registry;
		entt::entity m_player;
		bump::grid2<entt::entity, glm::ivec2> m_actors;

		std::optional<glm::ivec2> m_hovered_tile;
		std::vector<glm::ivec2> m_queued_path;
	};

	std::vector<glm::ivec2> find_path(bump::grid2<feature, glm::ivec2> const& grid, glm::ivec2 src, glm::ivec2 dst);

} // rog
