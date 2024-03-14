#pragma once

#include <bump_aabb.hpp>
#include <bump_math.hpp>

namespace rog
{

	struct c_player_char_info;
	struct c_xp;

	struct level;
	struct screen_buffer;

	// todo: other ui layouts... (birth, character screen, shops, inventory, etc.)

	struct ui_layout_main
	{
		bump::iaabb2 m_py_name_sb;
		bump::iaabb2 m_py_title_sb;
		bump::iaabb2 m_py_level_sb;
		bump::iaabb2 m_py_exp_sb;
		bump::iaabb2 m_py_stats_sb;
		bump::iaabb2 m_py_hp_sb;

		bump::iaabb2 m_msg_sb;
		bump::iaabb2 m_map_sb;
		bump::iaabb2 m_location_sb;
	};

	ui_layout_main calc_ui_layout_main(glm::ivec2 sb_size);

	void draw_player_char_info(screen_buffer& sb, c_player_char_info const& char_info, bump::iaabb2 const& name_panel_sb, bump::iaabb2 const& title_panel_sb);
	void draw_player_exp(screen_buffer& sb, c_xp const& exp, bump::iaabb2 const& lvl_panel_sb, bump::iaabb2 const& exp_panel_sb);
	void draw_level(screen_buffer& sb, level const& level, bump::iaabb2 const& map_panel_sb);

} // rog
