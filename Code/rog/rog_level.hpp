#pragma once

#include "rog_direction.hpp"

#include <bump_aabb.hpp>
#include <bump_grid.hpp>

#include <entt.hpp>

#include <cstdint>

namespace rog
{

	struct comp_position;
	struct feature;
	struct screen_buffer;

	struct level
	{
		glm::ivec2 size() const { return glm::ivec2(m_grid.extents()); }
		bool in_bounds(glm::ivec2 pos) const { return bump::iaabb2{ glm::ivec2(0), size() }.contains(pos); }

		bump::iaabb2 get_map_panel(glm::ivec2 panel_size, glm::ivec2 focus_lv) const;
		bump::iaabb2 get_map_panel(glm::ivec2 panel_size) const;

		void draw_map(screen_buffer& sb, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const;
		void draw_player(screen_buffer& screen, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const;
		void draw_monsters(screen_buffer& screen, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const;
		void draw_queued_path(screen_buffer& screen, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const;
		void draw_hovered_tile(screen_buffer& screen, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const;

		void draw(screen_buffer& sb, bump::iaabb2 const& map_panel_sb) const;

		std::int32_t m_depth;
		bump::grid2<feature> m_grid;

		entt::registry m_registry;
		entt::entity m_player;
		bump::grid2<entt::entity> m_actors;

		std::optional<glm::ivec2> m_hovered_tile;
		std::vector<glm::ivec2> m_queued_path;
	};

	bool is_walkable(level const& level, glm::ivec2 pos);
	bool is_occupied(level const& level, glm::ivec2 pos);

	bool move_actor(level& level, entt::entity entity, comp_position& pos, glm::ivec2 target);
	bool move_actor(level& level, entt::entity entity, comp_position& pos, direction dir);

	std::vector<glm::ivec2> find_path(bump::grid2<feature> const& grid, glm::ivec2 src, glm::ivec2 dst);

} // rog
