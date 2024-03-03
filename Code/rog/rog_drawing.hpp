#pragma once

#include "rog_screen.hpp"
#include "rog_tile_renderer.hpp"

#include <bump_grid.hpp>
#include <bump_math.hpp>

#include <optional>
#include <string>

namespace rog
{

	struct feature;
	struct level;
	struct player;
	
	glm::size2 get_panel_origin(glm::size2 level_size, glm::size2 panel_size, glm::size2 focus);

	void draw_level(screen& screen, level& level, std::vector<glm::size2> const& path, std::optional<glm::size2> hovered_tile);

	void prepare_instance_data(screen const& screen, glm::vec2 window_size, glm::vec2 tile_size, tile_instance_data& instances);

} // rog
