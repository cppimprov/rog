#include "rog_ui.hpp"

#include "rog_ecs.hpp"
#include "rog_level.hpp"

namespace rog
{

	ui_layout_main calc_ui_layout_main(glm::ivec2 sb_size)
	{
		auto const py_panel_width = 12;

		auto ui = ui_layout_main();

		ui.m_py_name_sb = { { 0, 0 }, { py_panel_width, 1 } };
		ui.m_py_title_sb = { { 0, ui.m_py_name_sb.m_origin.y + ui.m_py_name_sb.m_size.y }, { py_panel_width, 1 } };
		ui.m_py_level_sb = { { 0, ui.m_py_title_sb.m_origin.y + ui.m_py_title_sb.m_size.y }, { py_panel_width, 1 } };
		ui.m_py_exp_sb = { { 0, ui.m_py_level_sb.m_origin.y + ui.m_py_level_sb.m_size.y }, { py_panel_width, 1 } };
		ui.m_py_stats_sb = { { 0, ui.m_py_exp_sb.m_origin.y + ui.m_py_exp_sb.m_size.y + 1 }, { py_panel_width, 6 } };
		ui.m_py_hp_sb = { { 0, ui.m_py_stats_sb.m_origin.y + ui.m_py_stats_sb.m_size.y + 1 }, { py_panel_width, 2 } };

		ui.m_msg_sb = { { py_panel_width + 1, 0 }, { sb_size.x - py_panel_width - 1, 1 } };
		ui.m_location_sb = { { sb_size.x - 12, sb_size.y - 1 }, { 12, 1 } };
		ui.m_map_sb = { { py_panel_width + 1, 1 }, { sb_size.x - py_panel_width - 1, sb_size.y - 2 } };

		return ui;
	}

	enum class text_align
	{
		LEFT,
		CENTER,
		RIGHT
	};

	void draw_clipped_text(screen_buffer& sb, std::string const& text, bump::iaabb2 const& panel_sb, glm::vec3 fg_color, glm::vec3 bg_color)
	{
		if (text.empty())
			return;

		if (panel_sb.m_size.x < 1 || panel_sb.m_size.y < 1)
			return;

		auto x = 0;

		for (auto c : text)
		{
			auto const pos = panel_sb.m_origin + glm::ivec2{ x++, 0 };
			auto const cell = screen_cell{ static_cast<std::uint8_t>(c), fg_color, bg_color };
			sb.m_data.at(pos) = cell;

			if (x >= panel_sb.m_size.x)
				break;
		}
	}

	void draw_player_char_info(screen_buffer& sb, c_player_char_info const& n, bump::iaabb2 const& name_panel_sb, bump::iaabb2 const& title_panel_sb)
	{
		draw_clipped_text(sb, n.m_name, name_panel_sb, colors::white, colors::black);
		draw_clipped_text(sb, n.m_title, title_panel_sb, colors::white, colors::black);
	}

	void draw_player_exp(screen_buffer& sb, c_exp const& e, bump::iaabb2 const& lvl_panel_sb, bump::iaabb2 const& exp_panel_sb)
	{
		draw_clipped_text(sb, "LEVEL:", lvl_panel_sb, colors::white, colors::black);

		//auto const lvl_str = std::to_string(e.m_level);
		//draw_clipped_text(sb, lvl_str, lvl_panel_sb, colors::white, colors::black);

		draw_clipped_text(sb, "EXP:", exp_panel_sb, colors::white, colors::black);

		//auto const exp_str = std::to_string(e.m_exp);
		//draw_clipped_text(sb, exp_str, exp_panel_sb, colors::white, colors::black);
	}

	void draw_map(screen_buffer& sb, level const& level, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv)
	{
		auto const min = map_panel_lv.m_origin;
		auto const max = min + map_panel_lv.m_size;

		for (auto y : bump::range(min.y, max.y))
		{
			for (auto x : bump::range(min.x, max.x))
			{
				auto const pos_lv = glm::ivec2{ x, y };
				auto const pos_sb = panel_cell_to_buffer_cell(map_coords_to_panel_cell(pos_lv, map_panel_lv.m_origin), map_panel_sb.m_origin);
				sb.m_data.at(pos_sb) = level.m_grid.at(pos_lv).m_cell;
			}
		}
	}

	void draw_player(screen_buffer& screen, level const& level, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv)
	{
		auto const [pp, pv] = level.m_registry.get<c_position, c_visual>(level.m_player);

		if (!map_panel_lv.contains(pp.m_pos))
			return;

		auto const player_pos_pn = map_coords_to_panel_cell(pp.m_pos, map_panel_lv.m_origin);
		auto const player_pos_sb = panel_cell_to_buffer_cell(player_pos_pn, map_panel_sb.m_origin);
		screen.m_data.at(player_pos_sb) = pv.m_cell;
	}

	void draw_monsters(screen_buffer& screen, level const& level, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv)
	{
		auto view = level.m_registry.view<c_position const, c_visual const, c_monster_tag const>();

		for (auto const m : view)
		{
			auto [pos, vis] = view.get<c_position const, c_visual const>(m);

			auto const pos_lv = pos.m_pos;

			if (!map_panel_lv.contains(pos_lv))
				continue;

			auto const pos_pn = map_coords_to_panel_cell(pos.m_pos, map_panel_lv.m_origin);
			auto const pos_sb = panel_cell_to_buffer_cell(pos_pn, map_panel_sb.m_origin);
			screen.m_data.at(pos_sb) = vis.m_cell;
		}
	}

	void draw_queued_path(screen_buffer& screen, level const& level, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv)
	{
		for (auto const& p : level.m_queued_path)
		{
			if (!map_panel_lv.contains(p))
				continue;

			auto const p_pn = map_coords_to_panel_cell(p, map_panel_lv.m_origin);
			auto const p_sb = panel_cell_to_buffer_cell(p_pn, map_panel_sb.m_origin);
			screen.m_data.at(p_sb).m_bg = colors::dark_red;
		}
	}

	void draw_hovered_tile(screen_buffer& screen, level const& level, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv)
	{
		if (!level.m_hovered_tile.has_value())
			return;

		auto const& ht = level.m_hovered_tile.value();

		if (!map_panel_lv.contains(ht))
			return;

		auto const ht_pn = map_coords_to_panel_cell(ht, map_panel_lv.m_origin);
		auto const ht_sb = panel_cell_to_buffer_cell(ht_pn, map_panel_sb.m_origin);
		screen.m_data.at(ht_sb).m_bg = colors::orange;
	}

	void draw_level(screen_buffer& sb, level const& level, bump::iaabb2 const& map_panel_sb)
	{
		auto const map_panel_lv = level.get_map_panel(map_panel_sb.m_size);

		draw_map(sb, level, map_panel_sb, map_panel_lv);
		draw_player(sb, level, map_panel_sb, map_panel_lv);
		draw_monsters(sb, level, map_panel_sb, map_panel_lv);
		draw_queued_path(sb, level, map_panel_sb, map_panel_lv);
		draw_hovered_tile(sb, level, map_panel_sb, map_panel_lv);
	}

} // rog
