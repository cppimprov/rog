#include "rog_screen.hpp"

#include "rog_colors.hpp"
#include "rog_entity.hpp"
#include "rog_feature.hpp"
#include "rog_level.hpp"

#include <bump_aabb.hpp>
#include <bump_transform.hpp>

#include <glm/common.hpp>

namespace rog
{

	void screen_buffer::fill(screen_cell const& cell)
	{
		fill_rect(glm::ivec2(0), glm::ivec2(m_data.extents()), cell);
	}

	void screen_buffer::fill_rect(glm::ivec2 origin, glm::ivec2 size, screen_cell const& cell)
	{
		auto const extents = glm::ivec2(m_data.extents());
		auto const begin = glm::clamp(origin, glm::ivec2(0), extents);
		auto const end = glm::clamp(origin + size, begin, extents);

		for (auto y : bump::range(begin.y, end.y))
			for (auto x : bump::range(begin.x, end.x))
				m_data.at({ x, y }) = cell;
	}

	void screen_buffer::resize(glm::ivec2 size, screen_cell const& cell)
	{
		bump::die_if(size.x <= 0 || size.y <= 0);
		m_data.resize(glm::size2(size), cell);
	}

	tile_renderer::tile_renderer(bump::gl::shader_program const& shader, bump::gl::texture_2d_array const& texture):
		m_shader(&shader),
		m_texture(&texture),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_in_TilePosition(shader.get_attribute_location("in_TilePosition")),
		m_in_TileLayer(shader.get_attribute_location("in_TileLayer")),
		m_in_TileFGColor(shader.get_attribute_location("in_TileFGColor")),
		m_in_TileBGColor(shader.get_attribute_location("in_TileBGColor")),
		m_u_TileSize(shader.get_uniform_location("u_TileSize")),
		m_u_TileTexture(shader.get_uniform_location("u_TileTexture")),
		m_u_MVP(shader.get_uniform_location("u_MVP"))
	{
		auto const vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);

		m_tile_positions_buffer.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 2, 0, GL_STREAM_DRAW);
		m_vertex_array.set_array_buffer(m_in_TilePosition, m_tile_positions_buffer, 1);

		m_tile_layers_buffer.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 1, 0, GL_STREAM_DRAW);
		m_vertex_array.set_array_buffer(m_in_TileLayer, m_tile_layers_buffer, 1);
		
		m_tile_fg_colors_buffer.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
		m_vertex_array.set_array_buffer(m_in_TileFGColor, m_tile_fg_colors_buffer, 1);
		
		m_tile_bg_colors_buffer.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
		m_vertex_array.set_array_buffer(m_in_TileBGColor, m_tile_bg_colors_buffer, 1);
	}
	
	void tile_renderer::render(
		bump::gl::renderer& renderer,
		bump::camera_matrices const& matrices,
		tile_instance_data const& instances,
		glm::vec2 tile_size_px)
	{
		auto const instance_count = instances.positions.size();

		bump::die_if(instances.layers.size() != instance_count);
		bump::die_if(instances.fg_colors.size() != instance_count);
		bump::die_if(instances.bg_colors.size() != instance_count);

		if (instance_count == 0)
			return;

		m_tile_positions_buffer.set_data(GL_ARRAY_BUFFER, glm::value_ptr(instances.positions.front()), 2, instance_count, GL_STREAM_DRAW);
		m_tile_layers_buffer.set_data(GL_ARRAY_BUFFER, instances.layers.data(), 1, instance_count, GL_STREAM_DRAW);
		m_tile_fg_colors_buffer.set_data(GL_ARRAY_BUFFER, glm::value_ptr(instances.fg_colors.front()), 3, instance_count, GL_STREAM_DRAW);
		m_tile_bg_colors_buffer.set_data(GL_ARRAY_BUFFER, glm::value_ptr(instances.bg_colors.front()), 3, instance_count, GL_STREAM_DRAW);

		renderer.set_program(*m_shader);
		renderer.set_texture_2d_array(0, *m_texture);
		renderer.set_uniform_1i(m_u_TileTexture, 0);
		renderer.set_uniform_2f(m_u_TileSize, tile_size_px);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count(), instance_count);

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

	screen::screen(bump::gl::shader_program const& shader, bump::gl::texture_2d_array const& texture, glm::ivec2 window_size_px, glm::ivec2 tile_size_px):
		m_window_size_px(window_size_px),
		m_tile_size_px(tile_size_px),
		m_tile_renderer(shader, texture)
	{
		resize(window_size_px, tile_size_px);
	}

	void screen::resize(glm::ivec2 window_size_px, glm::ivec2 tile_size_px)
	{
		bump::die_if(window_size_px.x <= 0 || window_size_px.y <= 0);
		bump::die_if(tile_size_px.x <= 0 || tile_size_px.y <= 0);

		m_window_size_px = window_size_px;
		m_tile_size_px = tile_size_px;

		m_buffer.resize(window_size_px / tile_size_px, debug_cell);

		m_sb_size_px = tile_size_px * glm::ivec2(m_buffer.m_data.extents());
		m_sb_origin_px = (window_size_px - m_sb_size_px) / glm::ivec2(2);
	}

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
	std::size_t get_map_panel_origin(std::size_t level_size, std::size_t panel_size, std::size_t focus)
	{
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

	// todo: move these to the level class!
	glm::ivec2 get_map_panel_origin(glm::ivec2 level_size, glm::ivec2 panel_size, glm::ivec2 focus)
	{
		bump::die_if(level_size.x <= 0 || level_size.y <= 0);
		bump::die_if(panel_size.x <= 0 || panel_size.y <= 0);
		bump::die_if(focus.x < 0 || focus.y < 0);
		bump::die_if(focus.x >= level_size.x || focus.y >= level_size.y);

		return
		{ 
			get_map_panel_origin(level_size.x, panel_size.x, focus.x),
			get_map_panel_origin(level_size.y, panel_size.y, focus.y)
		};
	}

	namespace
	{

		void draw_map(screen_buffer& screen, level& level, glm::ivec2 panel_origin_sb, glm::ivec2 panel_origin_lv, glm::ivec2 panel_size_lv)
		{
			for (auto y : bump::range(0, panel_size_lv.y))
			{
				for (auto x : bump::range(0, panel_size_lv.x))
				{
					auto const pos = glm::ivec2{ x, y };
					auto const pos_sb = glm::size2(panel_origin_sb + pos);
					auto const pos_lv = glm::size2(panel_origin_lv + pos);
					screen.m_data.at(pos_sb) = level.m_grid.at(pos_lv).m_cell;
				}
			}
		}

		void draw_player(screen_buffer& screen, level& level, glm::ivec2 panel_origin_sb, glm::ivec2 panel_origin_lv, glm::ivec2 panel_size_lv)
		{
			auto const [pp, pv] = level.m_registry.get<comp_position, comp_visual>(level.m_player);

			if (!bump::iaabb2{ panel_origin_lv, panel_size_lv }.contains(pp.m_pos))
				return;

			auto const player_pos_pn = map_coords_to_panel_cell(pp.m_pos, panel_origin_lv);
			auto const player_pos_sb = panel_cell_to_buffer_cell(player_pos_pn, panel_origin_sb);
			screen.m_data.at(glm::size2(player_pos_sb)) = pv.m_cell;
		}

		void draw_monsters(screen_buffer& screen, level& level, glm::ivec2 panel_origin_sb, glm::ivec2 panel_origin_lv, glm::ivec2 panel_size_lv)
		{
			auto view = level.m_registry.view<comp_position, comp_visual, comp_monster_tag>();

			for (auto const m : view)
			{
				auto [pos, vis] = view.get<comp_position, comp_visual>(m);

				auto const pos_lv = pos.m_pos;

				if (!bump::iaabb2{ panel_origin_lv, panel_size_lv }.contains(pos_lv))
					continue;

				auto const pos_pn = map_coords_to_panel_cell(pos.m_pos, panel_origin_lv);
				auto const pos_sb = panel_cell_to_buffer_cell(pos_pn, panel_origin_sb);
				screen.m_data.at(glm::size2(pos_sb)) = vis.m_cell;
			}
		}

		void draw_queued_path(screen_buffer& screen, level& level, glm::ivec2 panel_origin_sb, glm::ivec2 panel_origin_lv, glm::ivec2 panel_size_lv)
		{
			for (auto const& p : level.m_queued_path)
			{
				if (!bump::iaabb2{ panel_origin_lv, panel_size_lv }.contains(p))
					continue;

				auto const p_pn = map_coords_to_panel_cell(p, panel_origin_lv);
				auto const p_sb = panel_cell_to_buffer_cell(p_pn, panel_origin_sb);
				screen.m_data.at(glm::size2(p_sb)).m_bg = colors::dark_red;
			}
		}

		void draw_hovered_tile(screen_buffer& screen, level& level, glm::ivec2 panel_origin_sb, glm::ivec2 panel_origin_lv, glm::ivec2 panel_size_lv)
		{
			if (!level.m_hovered_tile.has_value())
				return;

			auto const ht = level.m_hovered_tile.value();

			if (!bump::iaabb2{ panel_origin_lv, panel_size_lv }.contains(ht))
				return;

			auto const ht_pn = map_coords_to_panel_cell(ht, panel_origin_lv);
			auto const ht_sb = panel_cell_to_buffer_cell(ht_pn, panel_origin_sb);
			screen.m_data.at(glm::size2(ht_sb)).m_bg = colors::orange;
		}

	} // unnamed

	void screen::draw(level& level)
	{
		// todo: use actual panel coords / size
		auto const panel_origin_sb = glm::ivec2(0);
		auto const panel_size_sb = glm::ivec2(m_buffer.m_data.extents());

		auto const& player_pos_lv = level.m_registry.get<comp_position>(level.m_player).m_pos;

		// calculate origin of panel in level coords
		auto const panel_origin_lv = get_map_panel_origin(level.size(), panel_size_sb, player_pos_lv);
		auto const panel_max_lv = glm::min(panel_origin_lv + panel_size_sb, level.size());
		auto const panel_size_lv = (panel_max_lv - panel_origin_lv);

		draw_map(m_buffer, level, panel_origin_sb, panel_origin_lv, panel_size_lv);
		draw_player(m_buffer, level, panel_origin_sb, panel_origin_lv, panel_size_lv);
		draw_monsters(m_buffer, level, panel_origin_sb, panel_origin_lv, panel_size_lv);
		draw_queued_path(m_buffer, level, panel_origin_sb, panel_origin_lv, panel_size_lv);
		draw_hovered_tile(m_buffer, level, panel_origin_sb, panel_origin_lv, panel_size_lv);
	}

	namespace
	{

		bump::camera_matrices prepare_camera(glm::vec2 window_size_px)
		{
			auto camera = bump::orthographic_camera();

			camera.m_projection.m_size = window_size_px;
			camera.m_viewport.m_size = window_size_px;

			bump::rotate_around_local_axis(camera.m_transform, glm::vec3{ 1.f, 0.f, 0.f }, glm::radians(-90.f));

			return bump::camera_matrices(camera);
		}
		
		void prepare_instances(screen_buffer const& buffer, glm::ivec2 sb_origin_px, glm::ivec2 tile_size_px, glm::ivec2 sb_size_sb, tile_instance_data& instances)
		{
			instances.clear();
			instances.reserve(buffer.m_data.size());

			for (auto y : bump::range(0, buffer.m_data.extents().y))
			{
				for (auto x : bump::range(0, buffer.m_data.extents().x))
				{
					auto const& cell = buffer.m_data.at({ x, y });

					auto const pos_sb = glm::ivec2{ x, y };
					auto const pos_px = buffer_cell_to_screen_px(pos_sb, sb_origin_px, tile_size_px, sb_size_sb);
					
					instances.positions.push_back(glm::vec2(pos_px));
					instances.layers.push_back(static_cast<float>(cell.m_value));
					instances.fg_colors.push_back(cell.m_fg);
					instances.bg_colors.push_back(cell.m_bg);
				}
			}
		}

	} // unnamed

	void screen::render(bump::gl::renderer& renderer)
	{
		auto const matrices = prepare_camera(glm::vec2(m_window_size_px));
		prepare_instances(m_buffer, m_sb_origin_px, m_tile_size_px, m_buffer.size(), m_tile_instances);
		m_tile_renderer.render(renderer, matrices, m_tile_instances, glm::vec2(m_tile_size_px));
	}

} // rog
