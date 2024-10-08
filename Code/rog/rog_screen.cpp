#include "rog_screen.hpp"

#include "rog_colors.hpp"
#include "rog_ecs.hpp"
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
		m_data.resize(size, cell);
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

	tile_border_renderable::tile_border_renderable(bump::gl::shader_program const& shader):
		m_shader(&shader),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_in_VertexLerp(shader.get_attribute_location("in_VertexLerp")),
		m_in_BorderPosition(shader.get_attribute_location("in_BorderPosition")),
		m_in_BorderWidth(shader.get_attribute_location("in_BorderWidth")),
		m_in_BorderColor(shader.get_attribute_location("in_BorderColor")),
		m_u_TileSize(shader.get_uniform_location("u_TileSize")),
		m_u_MVP(shader.get_uniform_location("u_MVP"))
	{
		auto const vertices =
		{
			0.f, 0.f,  1.f, 0.f,  1.f, 0.f,  0.f, 0.f,  1.f, 0.f,  0.f, 0.f,  
			1.f, 0.f,  1.f, 1.f,  1.f, 1.f,  1.f, 0.f,  1.f, 1.f,  1.f, 0.f,  
			1.f, 1.f,  0.f, 1.f,  0.f, 1.f,  1.f, 1.f,  0.f, 1.f,  1.f, 1.f,  
			0.f, 1.f,  0.f, 0.f,  0.f, 0.f,  0.f, 1.f,  0.f, 0.f,  0.f, 1.f,  
		};

		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 24, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);

		auto const vertex_lerp =
		{
			0.f, 0.f, 1.f, 0.f, 1.f, 1.f,
			0.f, 0.f, 1.f, 0.f, 1.f, 1.f,
			0.f, 0.f, 1.f, 0.f, 1.f, 1.f,
			0.f, 0.f, 1.f, 0.f, 1.f, 1.f,
		};

		m_vertex_lerp_buffer.set_data(GL_ARRAY_BUFFER, vertex_lerp.begin(), 1, 24, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexLerp, m_vertex_lerp_buffer);

		m_border_positions_buffer.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 2, 0, GL_STREAM_DRAW);
		m_vertex_array.set_array_buffer(m_in_BorderPosition, m_border_positions_buffer, 1);

		m_border_widths_buffer.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 1, 0, GL_STREAM_DRAW);
		m_vertex_array.set_array_buffer(m_in_BorderWidth, m_border_widths_buffer, 1);
		
		m_border_colors_buffer.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 3, 0, GL_STREAM_DRAW);
		m_vertex_array.set_array_buffer(m_in_BorderColor, m_border_colors_buffer, 1);
	}

	void tile_border_renderable::render(
		bump::gl::renderer& renderer,
		bump::camera_matrices const& matrices,
		tile_border_instance_data const& instances,
		glm::vec2 tile_size_px)
	{
		auto const instance_count = instances.positions.size();

		bump::die_if(instances.widths.size() != instance_count);
		bump::die_if(instances.colors.size() != instance_count);

		if (instance_count == 0)
			return;

		m_border_positions_buffer.set_data(GL_ARRAY_BUFFER, glm::value_ptr(instances.positions.front()), 2, instance_count, GL_STREAM_DRAW);
		m_border_widths_buffer.set_data(GL_ARRAY_BUFFER, &instances.widths.front(), 1, instance_count, GL_STREAM_DRAW);
		m_border_colors_buffer.set_data(GL_ARRAY_BUFFER, glm::value_ptr(instances.colors.front()), 3, instance_count, GL_STREAM_DRAW);

		renderer.set_program(*m_shader);
		renderer.set_uniform_2f(m_u_TileSize, tile_size_px);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count(), instance_count);

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

	screen::screen(
		bump::gl::shader_program const& tile_shader, bump::gl::texture_2d_array const& texture, 
		bump::gl::shader_program const& border_shader,
		glm::ivec2 window_size_px, glm::ivec2 tile_size_px):
		m_window_size_px(window_size_px),
		m_tile_size_px(tile_size_px),
		m_tile_renderable(tile_shader, texture),
		m_tile_border_renderable(border_shader)
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
		
		void prepare_tile_instances(screen_buffer const& buffer, bump::iaabb2 const& sb_area_px, glm::ivec2 tile_size_px, glm::ivec2 sb_size_sb, tile_instance_data& instances)
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

		void prepare_tile_border_instances(screen_buffer const& buffer, bump::iaabb2 const& sb_area_px, glm::ivec2 tile_size_px, glm::ivec2 sb_size_sb, tile_border_instance_data& instances)
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

					if (cell.m_border_width > 0)
					{
						instances.positions.push_back(glm::vec2(pos_px));
						instances.widths.push_back(static_cast<float>(cell.m_border_width));
						instances.colors.push_back(cell.m_border);
					}
				}
			}
		}

	} // unnamed

	void screen::render(bump::gl::renderer& renderer)
	{
		auto const matrices = prepare_camera(glm::vec2(m_window_size_px));

		prepare_tile_instances(m_buffer, m_sb_area_px, m_tile_size_px, m_buffer.size(), m_tile_instances);
		m_tile_renderable.render(renderer, matrices, m_tile_instances, glm::vec2(m_tile_size_px));

		prepare_tile_border_instances(m_buffer, m_sb_area_px, m_tile_size_px, m_buffer.size(), m_tile_border_instances);
		m_tile_border_renderable.render(renderer, matrices, m_tile_border_instances, glm::vec2(m_tile_size_px));
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
