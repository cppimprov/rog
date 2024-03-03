#include "rog_drawing.hpp"

#include "rog_colors.hpp"
#include "rog_entity.hpp"
#include "rog_feature.hpp"
#include "rog_level.hpp"
#include "rog_player.hpp"

#include <algorithm>

namespace rog
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
	std::size_t get_panel_origin(std::size_t level_size, std::size_t panel_size, std::size_t focus)
	{
		bump::die_if(level_size == 0); // empty level, no valid origin
		bump::die_if(panel_size == 0); // division by zero
		bump::die_if(focus >= level_size); // focus outside of level

		if (level_size <= panel_size)
			return 0;

		auto const border_size = panel_size / std::size_t{ 4 };
		auto const half_panel_size = panel_size / std::size_t{ 2 };

		auto panel_origin = (focus / panel_size) * panel_size;
		auto const panel_focus = focus % panel_size;

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

	glm::size2 get_panel_origin(glm::size2 level_size, glm::size2 panel_size, glm::size2 focus)
	{
		return
		{ 
			get_panel_origin(level_size.x, panel_size.x, focus.x),
			get_panel_origin(level_size.y, panel_size.y, focus.y)
		};
	}

	void draw_level(screen& screen, level& level, std::vector<glm::size2> const& path, std::optional<glm::size2> hovered_tile)
	{
		auto const level_size = level.m_grid.extents();
		auto const panel_size = glm::size2(screen.m_buffer.extents()); // todo: no! pass it in!

		auto const& player_pos = level.m_registry.get<comp_position>(level.m_player).m_pos;
		auto const& player_cell = level.m_registry.get<comp_visual>(level.m_player).m_cell;

		// calculate origin of screen buffer area in level coords
		auto const panel_origin = get_panel_origin(level_size, panel_size, player_pos);

		auto const panel_max = glm::min(panel_origin + panel_size, level_size);
		auto const panel_max_size = (panel_max - panel_origin);

		// blit level to screen
		for (auto y : bump::range(0, panel_max_size.y))
		{
			for (auto x : bump::range(0, panel_max_size.x))
			{
				auto const screen_pos = glm::size2{ x, y };
				screen.m_buffer.at(screen_pos) = level.m_grid.at(panel_origin + screen_pos).m_cell;
			}
		}
		
		// draw player
		auto const panel_player = (player_pos - panel_origin);
		screen.m_buffer.at(panel_player) = player_cell;

		// draw monsters
		auto view = level.m_registry.view<comp_position, comp_visual, comp_monster_tag>();

		for (auto const m : view)
		{
			auto [pos, vis] = view.get<comp_position, comp_visual>(m);
			screen.m_buffer.at(pos.m_pos - panel_origin) = vis.m_cell;
		}

		for (auto const& p : path)
		{
			screen.m_buffer.at(p - panel_origin).m_bg = colors::dark_red;
		}

		if (hovered_tile.has_value())
		{
			screen.m_buffer.at(hovered_tile.value() - panel_origin).m_bg = colors::orange;
		}
	}

	void prepare_instance_data(screen const& screen, glm::vec2 window_size, glm::vec2 tile_size, tile_instance_data& instances)
	{
		instances.clear();
		instances.reserve(screen.m_buffer.size());

		for (auto y : bump::range(0, screen.m_buffer.extents().y))
		{
			for (auto x : bump::range(0, screen.m_buffer.extents().x))
			{
				auto const& cell = screen.m_buffer.at({ x, y });
				
				// convert coord origin to top-left
				auto const pos = glm::vec2(x, y) * tile_size;
				auto const window_pos = glm::vec2(pos.x, (window_size.y - tile_size.y) - pos.y);

				instances.positions.push_back(window_pos);
				instances.layers.push_back(static_cast<float>(cell.m_value));
				instances.fg_colors.push_back(cell.m_fg);
				instances.bg_colors.push_back(cell.m_bg);
			}
		}
	}

} // rog
