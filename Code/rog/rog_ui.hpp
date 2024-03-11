#pragma once

#include <bump_aabb.hpp>
#include <bump_math.hpp>

namespace rog
{

	// todo: others... (birth, character screen, shops, inventory, etc.)

	struct ui_layout_main
	{
		bump::iaabb2 m_py_name_sb;
		bump::iaabb2 m_py_info_sb;
		bump::iaabb2 m_py_stats_sb;
		bump::iaabb2 m_py_hp_sb;

		bump::iaabb2 m_msg_sb;
		bump::iaabb2 m_map_sb;
		bump::iaabb2 m_location_sb;
	};

	ui_layout_main calc_ui_layout_main(glm::ivec2 sb_size);

} // rog