#include "rog_level.hpp"

#include "rog_direction.hpp"
#include "rog_ecs.hpp"
#include "rog_feature.hpp"
#include "rog_screen.hpp"

namespace rog
{

	bool level::is_walkable(glm::ivec2 pos) const
	{
		return !(m_grid.at(pos).m_flags & feature::flags::NO_WALK);
	}

	bool level::is_occupied(glm::ivec2 pos) const
	{
		return m_actors.at(pos) != entt::null;
	}
	
	bool level::move_actor(entt::entity entity, c_position& pos, glm::ivec2 target)
	{
		bump::die_if(!in_bounds(pos.m_pos));

		if (!in_bounds(target))
			return false;

		if (!is_walkable(target) || is_occupied(target))
			return false;

		m_actors.at(pos.m_pos) = entt::null;
		pos.m_pos = target;
		m_actors.at(pos.m_pos) = entity;

		return true;
	}

	bool level::move_actor(entt::entity entity, c_position& pos, direction dir)
	{
		return move_actor(entity, pos, pos.m_pos + get_direction_vector(dir));
	}

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
		return get_map_panel(panel_size, m_registry.get<c_position>(m_player).m_pos);
	}

} // rog
