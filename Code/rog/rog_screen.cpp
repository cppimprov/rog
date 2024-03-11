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

	tile_renderable::tile_renderable(bump::gl::shader_program const& shader, bump::gl::texture_2d_array const& texture):
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
	
	void tile_renderable::render(
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
		m_tile_renderable(shader, texture)
	{
		resize(window_size_px, tile_size_px);
	}

	void screen::resize(glm::ivec2 window_size_px, glm::ivec2 tile_size_px)
	{
		bump::die_if(window_size_px.x <= 0 || window_size_px.y <= 0);
		bump::die_if(tile_size_px.x <= 0 || tile_size_px.y <= 0);

		m_window_size_px = window_size_px;
		m_tile_size_px = tile_size_px;

		m_buffer.resize(window_size_px / tile_size_px, screen_cell_debug);

		auto const sb_size_px = tile_size_px * glm::ivec2(m_buffer.m_data.extents());
		m_sb_area_px = { (window_size_px - sb_size_px) / glm::ivec2(2), sb_size_px };
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
		
		void prepare_instances(screen_buffer const& buffer, bump::iaabb2 const& sb_area_px, glm::ivec2 tile_size_px, glm::ivec2 sb_size_sb, tile_instance_data& instances)
		{
			instances.clear();
			instances.reserve(buffer.m_data.size());

			for (auto y : bump::range(0, buffer.m_data.extents().y))
			{
				for (auto x : bump::range(0, buffer.m_data.extents().x))
				{
					auto const& cell = buffer.m_data.at({ x, y });

					auto const pos_sb = glm::ivec2{ x, y };
					auto const pos_px = buffer_cell_to_screen_px(pos_sb, sb_area_px.m_origin, tile_size_px, sb_size_sb);
					
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
		prepare_instances(m_buffer, m_sb_area_px, m_tile_size_px, m_buffer.size(), m_tile_instances);
		m_tile_renderable.render(renderer, matrices, m_tile_instances, glm::vec2(m_tile_size_px));
	}

	ui_layout calc_ui_layout(glm::ivec2 sb_size)
	{
		auto const py_panel_width = 12;

		auto ui = ui_layout();

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
