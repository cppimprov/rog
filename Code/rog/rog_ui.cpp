#include "rog_ui.hpp"

namespace rog
{

	ui_layout_main calc_ui_layout_main(glm::ivec2 sb_size)
	{
		auto const py_panel_width = 12;

		auto ui = ui_layout_main();

		ui.m_py_name_sb = { { 0, 0 }, { py_panel_width, 1 } };
		ui.m_py_info_sb = { { 0, ui.m_py_name_sb.m_origin.y + ui.m_py_name_sb.m_size.y + 1 }, { py_panel_width, 3 } };
		ui.m_py_stats_sb = { { 0, ui.m_py_info_sb.m_origin.y + ui.m_py_info_sb.m_size.y + 1 }, { py_panel_width, 6 } };
		ui.m_py_hp_sb = { { 0, ui.m_py_stats_sb.m_origin.y + ui.m_py_stats_sb.m_size.y + 1 }, { py_panel_width, 2 } };

		ui.m_msg_sb = { { py_panel_width + 1, 0 }, { sb_size.x - py_panel_width - 1, 1 } };
		ui.m_location_sb = { { sb_size.x - 12, sb_size.y - 1 }, { 12, 1 } };
		ui.m_map_sb = { { py_panel_width + 1, 1 }, { sb_size.x - py_panel_width - 1, sb_size.y - 2 } };

		return ui;
	}

} // rog
