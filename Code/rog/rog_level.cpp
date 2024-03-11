#include "rog_level.hpp"

#include "rog_direction.hpp"
#include "rog_entity.hpp"
#include "rog_feature.hpp"
#include "rog_screen.hpp"

namespace rog
{

	namespace
	{

		/* get_panel_origin()
		* 
		* Calculates the origin of a viewing rectangle of `panel_size` in level
		* coordinates "centered" on the point `focus` (also level coordinates).
		* 
		* If the level is smaller than the panel { 0, 0 } is returned (so it
		* will be displayed in the top left.
		* 
		* Modular arithmetic is used to find the panel that covers the `focus`
		* point. Then, if the `focus` is near the edge of the panel, the panel
		* origin is moved by 1/2 `panel_size` to place it nearer the center.
		* Thus `focus` should be kept in the middle 1/2 of the viewing panel.
		* 
		*/
		std::size_t get_map_panel_origin(std::size_t level_size, std::size_t panel_size, std::size_t focus_lv)
		{
			if (level_size <= panel_size)
				return 0;

			auto const border_size = panel_size / std::size_t{ 4 };
			auto const half_panel_size = panel_size / std::size_t{ 2 };

			auto panel_origin = (focus_lv / panel_size) * panel_size;
			auto const panel_focus = focus_lv % panel_size;

			// handle scrolling in border region
			if (panel_focus <= border_size)
			{
				// subtract half panel size (or set to zero)
				panel_origin = panel_origin > half_panel_size ? panel_origin - half_panel_size : 0;
			}
			else if ((panel_size - 1) - panel_focus <= border_size)
			{
				// add half panel size
				panel_origin = panel_origin + half_panel_size;
			}

			// keep the bottom right of the level at the bottom right of the screen
			auto const max_panel_origin = level_size - panel_size;
			panel_origin = std::min(panel_origin, max_panel_origin);
			
			return panel_origin;
		}

	} // unnamed

	bump::iaabb2 level::get_map_panel(glm::ivec2 panel_size, glm::ivec2 focus_lv) const
	{
		bump::die_if(!in_bounds(focus_lv));

		auto const level_size = size();

		auto const origin_lv = glm::ivec2{ get_map_panel_origin(level_size.x, panel_size.x, focus_lv.x), get_map_panel_origin(level_size.y, panel_size.y, focus_lv.y) };
		auto const size_lv = glm::min(panel_size, level_size - origin_lv);

		return { origin_lv, size_lv };
	}

	bump::iaabb2 level::get_map_panel(glm::ivec2 panel_size) const
	{
		return get_map_panel(panel_size, m_registry.get<comp_position>(m_player).m_pos);
	}

	void level::draw_map(screen_buffer& sb, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const
	{
		auto const min = map_panel_lv.m_origin;
		auto const max = min + map_panel_lv.m_size;

		for (auto y : bump::range(min.y, max.y))
		{
			for (auto x : bump::range(min.x, max.x))
			{
				auto const pos_lv = glm::ivec2{ x, y };
				auto const pos_sb = panel_cell_to_buffer_cell(map_coords_to_panel_cell(pos_lv, map_panel_lv.m_origin), map_panel_sb.m_origin);
				sb.m_data.at(glm::size2(pos_sb)) = m_grid.at(glm::size2(pos_lv)).m_cell;
			}
		}
	}

	void level::draw_player(screen_buffer& screen, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const
	{
		auto const [pp, pv] = m_registry.get<comp_position, comp_visual>(m_player);

		if (!map_panel_lv.contains(pp.m_pos))
			return;

		auto const player_pos_pn = map_coords_to_panel_cell(pp.m_pos, map_panel_lv.m_origin);
		auto const player_pos_sb = panel_cell_to_buffer_cell(player_pos_pn, map_panel_sb.m_origin);
		screen.m_data.at(glm::size2(player_pos_sb)) = pv.m_cell;
	}

	void level::draw_monsters(screen_buffer& screen, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const
	{
		auto view = m_registry.view<comp_position const, comp_visual const, comp_monster_tag const>();

		for (auto const m : view)
		{
			auto [pos, vis] = view.get<comp_position const, comp_visual const>(m);

			auto const pos_lv = pos.m_pos;

			if (!map_panel_lv.contains(pos_lv))
				continue;

			auto const pos_pn = map_coords_to_panel_cell(pos.m_pos, map_panel_lv.m_origin);
			auto const pos_sb = panel_cell_to_buffer_cell(pos_pn, map_panel_sb.m_origin);
			screen.m_data.at(glm::size2(pos_sb)) = vis.m_cell;
		}
	}

	void level::draw_queued_path(screen_buffer& screen, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const
	{
		for (auto const& p : m_queued_path)
		{
			if (!map_panel_lv.contains(p))
				continue;

			auto const p_pn = map_coords_to_panel_cell(p, map_panel_lv.m_origin);
			auto const p_sb = panel_cell_to_buffer_cell(p_pn, map_panel_sb.m_origin);
			screen.m_data.at(glm::size2(p_sb)).m_bg = colors::dark_red;
		}
	}

	void level::draw_hovered_tile(screen_buffer& screen, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv) const
	{
		if (!m_hovered_tile.has_value())
			return;

		auto const& ht = m_hovered_tile.value();

		if (!map_panel_lv.contains(ht))
			return;

		auto const ht_pn = map_coords_to_panel_cell(ht, map_panel_lv.m_origin);
		auto const ht_sb = panel_cell_to_buffer_cell(ht_pn, map_panel_sb.m_origin);
		screen.m_data.at(glm::size2(ht_sb)).m_bg = colors::orange;
	}

	void level::draw(screen_buffer& sb, bump::iaabb2 const& map_panel_sb) const
	{
		auto const map_panel_lv = get_map_panel(map_panel_sb.m_size);

		draw_map(sb, map_panel_sb, map_panel_lv);
		draw_player(sb, map_panel_sb, map_panel_lv);
		draw_monsters(sb, map_panel_sb, map_panel_lv);
		draw_queued_path(sb, map_panel_sb, map_panel_lv);
		draw_hovered_tile(sb, map_panel_sb, map_panel_lv);
	}
	
	bool is_walkable(level const& level, glm::ivec2 pos)
	{
		return !(level.m_grid.at(glm::size2(pos)).m_flags & feature::flags::NO_WALK);
	}

	bool is_occupied(level const& level, glm::ivec2 pos)
	{
		return level.m_actors.at(glm::size2(pos)) != entt::null;
	}

	bool move_actor(level& level, entt::entity entity, comp_position& pos, glm::ivec2 target)
	{
		auto const level_size = level.size();

		bump::die_if(level_size.x == 0 || level_size.y == 0);
		bump::die_if(pos.m_pos.x >= level_size.x || pos.m_pos.y >= level_size.y);
		
		if (target.x >= level_size.x) return false;
		if (target.y >= level_size.y) return false;

		if (!is_walkable(level, target) || is_occupied(level, target)) return false;

		level.m_actors.at(glm::size2(pos.m_pos)) = entt::null;
		pos.m_pos = target;
		level.m_actors.at(glm::size2(pos.m_pos)) = entity;

		return true;
	}

	bool move_actor(level& level, entt::entity entity, comp_position& pos, direction dir)
	{
		auto const vec = get_direction_vector(dir);
		auto const level_size = level.size();

		bump::die_if(level_size.x == 0 || level_size.y == 0);
		bump::die_if(pos.m_pos.x >= level_size.x || pos.m_pos.y >= level_size.y);

		if (pos.m_pos.x == 0 && vec.x < 0) return false;
		if (pos.m_pos.x == level_size.x - 1 && vec.x > 0) return false;
		if (pos.m_pos.y == 0 && vec.y < 0) return false;
		if (pos.m_pos.y == level_size.y - 1 && vec.y > 0) return false;

		auto const target = pos.m_pos + vec;

		if (!is_walkable(level, target) || is_occupied(level, target)) return false;

		level.m_actors.at(glm::size2(pos.m_pos)) = entt::null;
		pos.m_pos = target;
		level.m_actors.at(glm::size2(pos.m_pos)) = entity;

		return true;
	}

} // rog
