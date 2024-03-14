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
		ui.m_py_hp_sb = { { 0, ui.m_py_stats_sb.m_origin.y + ui.m_py_stats_sb.m_size.y + 1 }, { py_panel_width, 1 } };
		ui.m_py_sp_sb = { { 0, ui.m_py_hp_sb.m_origin.y + ui.m_py_hp_sb.m_size.y }, { py_panel_width, 1 } };

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

	int align_text(int length, int clip_length, text_align align)
	{
		return
		    align == text_align::LEFT ? 0 :
		    align == text_align::CENTER ? (length - clip_length) / 2 :
		    align == text_align::RIGHT ? length - clip_length :
		    (bump::die(), 0);
	}

	void draw_clipped_text(screen_buffer& sb, std::string const& text, bump::iaabb2 const& panel_sb, glm::vec3 fg_color, glm::vec3 bg_color, text_align align = text_align::LEFT)
	{
		if (text.empty())
			return;

		if (panel_sb.m_size.x < 1 || panel_sb.m_size.y < 1)
			return;

		auto const length = static_cast<int>(text.size());
		auto const clip_length = std::min(length, panel_sb.m_size.x);

		auto const start_sb = panel_sb.m_origin.x + align_text(panel_sb.m_size.x, clip_length, align);
		auto const end_sb = start_sb + clip_length;

		auto const start_t = align_text(length, clip_length, align);
		auto const end_t = start_t + clip_length;

		for (auto x = start_sb, i = start_t; x != end_sb && i != end_t; ++x, ++i)
		{
			auto const pos = glm::ivec2{ x, panel_sb.m_origin.y };

			if (!sb.in_bounds(pos))
				continue;

			sb.m_data.at(pos) = screen_cell{ static_cast<std::uint8_t>(text[i]), fg_color, bg_color };
		}
	}

	void draw_player_char_info(screen_buffer& sb, c_player_char_info const& n, bump::iaabb2 const& name_panel_sb, bump::iaabb2 const& title_panel_sb)
	{
		draw_clipped_text(sb, n.m_name, name_panel_sb, colors::white, colors::black, text_align::LEFT);
		draw_clipped_text(sb, n.m_title, title_panel_sb, colors::white, colors::black);
	}
	
	void draw_player_xp(screen_buffer& sb, c_xp const& xp, bump::iaabb2 const& lvl_panel_sb, bump::iaabb2 const& xp_panel_sb)
	{
		auto const lvl_str = std::to_string(xp.m_level);
		draw_clipped_text(sb, "LEVEL:", lvl_panel_sb, colors::white, colors::black);
		draw_clipped_text(sb, lvl_str, lvl_panel_sb, colors::light_green, colors::black, text_align::RIGHT);

		auto const exp_str = std::to_string(xp.m_xp);
		draw_clipped_text(sb, "XP:", xp_panel_sb, colors::white, colors::black);
		draw_clipped_text(sb, exp_str, xp_panel_sb, colors::light_green, colors::black, text_align::RIGHT);
	}

	void draw_player_stats(screen_buffer& sb, c_stats const& stats, bump::iaabb2 const& stats_panel_sb)
	{
		auto const origin = stats_panel_sb.m_origin;
		auto const size = stats_panel_sb.m_size;

		auto const str_str = std::to_string(stats.m_str);
		auto const str_ofs = glm::ivec2{ 0, 0 };
		draw_clipped_text(sb, "STR: ", { origin + str_ofs, size }, colors::white, colors::black);
		draw_clipped_text(sb, str_str, { origin + str_ofs, size }, colors::light_green, colors::black, text_align::RIGHT);

		auto const dex_str = std::to_string(stats.m_dex);
		auto const dex_ofs = glm::ivec2{ 0, 1 };
		draw_clipped_text(sb, "DEX: ", { origin + dex_ofs, size }, colors::white, colors::black);
		draw_clipped_text(sb, dex_str, { origin + dex_ofs, size }, colors::light_green, colors::black, text_align::RIGHT);

		auto const con_str = std::to_string(stats.m_con);
		auto const con_ofs = glm::ivec2{ 0, 2 };
		draw_clipped_text(sb, "CON: ", { origin + con_ofs, size }, colors::white, colors::black);
		draw_clipped_text(sb, con_str, { origin + con_ofs, size }, colors::light_green, colors::black, text_align::RIGHT);

		auto const int_str = std::to_string(stats.m_int);
		auto const int_ofs = glm::ivec2{ 0, 3 };
		draw_clipped_text(sb, "INT: ", { origin + int_ofs, size }, colors::white, colors::black);
		draw_clipped_text(sb, int_str, { origin + int_ofs, size }, colors::light_green, colors::black, text_align::RIGHT);

		auto const wis_str = std::to_string(stats.m_wis);
		auto const wis_ofs = glm::ivec2{ 0, 4 };
		draw_clipped_text(sb, "WIS: ", { origin + wis_ofs, size }, colors::white, colors::black);
		draw_clipped_text(sb, wis_str, { origin + wis_ofs, size }, colors::light_green, colors::black, text_align::RIGHT);

		auto const cha_str = std::to_string(stats.m_cha);
		auto const cha_ofs = glm::ivec2{ 0, 5 };
		draw_clipped_text(sb, "CHA: ", { origin + cha_ofs, size }, colors::white, colors::black);
		draw_clipped_text(sb, cha_str, { origin + cha_ofs, size }, colors::light_green, colors::black, text_align::RIGHT);
	}
	
	void draw_player_hp(screen_buffer& sb, c_hp const& hp, bump::iaabb2 const& hp_panel_sb)
	{
		auto const hp_max_str = std::to_string(hp.m_max) + std::string("/    ");
		auto const hp_cur_str = std::to_string(hp.m_current);
		draw_clipped_text(sb, "HP:", hp_panel_sb, colors::light_red, colors::black);
		draw_clipped_text(sb, hp_max_str, hp_panel_sb, colors::light_green, colors::black, text_align::RIGHT);
		draw_clipped_text(sb, hp_cur_str, hp_panel_sb, colors::light_green, colors::black, text_align::RIGHT);
	}

	void draw_player_sp(screen_buffer& sb, c_sp const& sp, bump::iaabb2 const& sp_panel_sb)
	{
		auto const sp_max_str = std::to_string(sp.m_max) + std::string("/    ");
		auto const sp_cur_str = std::to_string(sp.m_current);
		draw_clipped_text(sb, "SP:", sp_panel_sb, colors::light_blue, colors::black);
		draw_clipped_text(sb, sp_max_str, sp_panel_sb, colors::light_green, colors::black, text_align::RIGHT);
		draw_clipped_text(sb, sp_cur_str, sp_panel_sb, colors::light_green, colors::black, text_align::RIGHT);
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

	void draw_player(screen_buffer& sb, level const& level, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv)
	{
		auto const [pp, pv] = level.m_registry.get<c_position, c_visual>(level.m_player);

		if (!map_panel_lv.contains(pp.m_pos))
			return;

		auto const player_pos_pn = map_coords_to_panel_cell(pp.m_pos, map_panel_lv.m_origin);
		auto const player_pos_sb = panel_cell_to_buffer_cell(player_pos_pn, map_panel_sb.m_origin);
		sb.m_data.at(player_pos_sb) = pv.m_cell;
	}

	void draw_monsters(screen_buffer& sb, level const& level, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv)
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
			sb.m_data.at(pos_sb) = vis.m_cell;
		}
	}

	void draw_queued_path(screen_buffer& sb, level const& level, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv)
	{
		for (auto const& p : level.m_queued_path)
		{
			if (!map_panel_lv.contains(p))
				continue;

			auto const p_pn = map_coords_to_panel_cell(p, map_panel_lv.m_origin);
			auto const p_sb = panel_cell_to_buffer_cell(p_pn, map_panel_sb.m_origin);
			sb.m_data.at(p_sb).m_bg = colors::dark_red;
		}
	}

	void draw_hovered_tile(screen_buffer& sb, level const& level, bump::iaabb2 const& map_panel_sb, bump::iaabb2 const& map_panel_lv)
	{
		if (!level.m_hovered_tile.has_value())
			return;

		auto const& ht = level.m_hovered_tile.value();

		if (!map_panel_lv.contains(ht))
			return;

		auto const ht_pn = map_coords_to_panel_cell(ht, map_panel_lv.m_origin);
		auto const ht_sb = panel_cell_to_buffer_cell(ht_pn, map_panel_sb.m_origin);
		sb.m_data.at(ht_sb).m_bg = colors::orange;
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
