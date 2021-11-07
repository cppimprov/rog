#include "rog_tile_renderer.hpp"

#include <bump_render_text.hpp>
#include <bump_transform.hpp>

namespace rog
{
	
	tile_renderer::tile_renderable::tile_renderable(bump::gl::shader_program const& shader):
		m_shader(&shader),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_in_TilePosition(shader.get_attribute_location("in_TilePosition")),
		m_in_TileLayer(shader.get_attribute_location("in_TileLayer")),
		m_in_TileFGColor(shader.get_attribute_location("in_TileFGColor")),
		m_in_TileBGColor(shader.get_attribute_location("in_TileBGColor")),
		m_u_TileSize(shader.get_uniform_location("u_TileSize")),
		m_u_TileTexture(shader.get_uniform_location("u_TileTexture")),
		m_u_MVP(shader.get_uniform_location("u_MVP"))
	{
		auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
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
	
	void tile_renderer::tile_renderable::render(bump::gl::renderer& renderer, 
		bump::camera_matrices const& matrices,
		bump::gl::texture_2d_array const& texture,
		std::vector<glm::vec2> const& positions,
		std::vector<float> const& layers,
		std::vector<glm::vec3> const& fg_colors,
		std::vector<glm::vec3> const& bg_colors,
		glm::vec2 tile_size)
	{
		auto const instance_count = positions.size();

		bump::die_if(layers.size() != instance_count);
		bump::die_if(fg_colors.size() != instance_count);
		bump::die_if(bg_colors.size() != instance_count);

		if (instance_count == 0)
			return;

		m_tile_positions_buffer.set_data(GL_ARRAY_BUFFER, glm::value_ptr(positions.front()), 2, instance_count, GL_STREAM_DRAW);
		m_tile_layers_buffer.set_data(GL_ARRAY_BUFFER, layers.data(), 1, instance_count, GL_STREAM_DRAW);
		m_tile_fg_colors_buffer.set_data(GL_ARRAY_BUFFER, glm::value_ptr(fg_colors.front()), 3, instance_count, GL_STREAM_DRAW);
		m_tile_bg_colors_buffer.set_data(GL_ARRAY_BUFFER, glm::value_ptr(bg_colors.front()), 3, instance_count, GL_STREAM_DRAW);

		renderer.set_program(*m_shader);
		renderer.set_texture_2d_array(0, texture);
		renderer.set_uniform_1i(m_u_TileTexture, 0);
		renderer.set_uniform_2f(m_u_TileSize, tile_size);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count(), instance_count);

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

	tile_renderer::tile_renderer(bump::app& app, glm::vec2 tile_size):
		m_tile_renderable(app.m_assets.m_shaders.at("tile")),
		m_tile_size(tile_size),
		m_tile_texture(&app.m_assets.m_texture_2d_arrays.at("ascii_tiles"))
	{

	}

	void tile_renderer::render(bump::gl::renderer& renderer, glm::vec2 window_size, screen::buffer const& screen)
	{
		auto camera = bump::orthographic_camera();
		camera.m_projection.m_size = window_size;
		camera.m_viewport.m_size = window_size;
		bump::rotate_around_local_axis(camera.m_transform, glm::vec3{ 1.f, 0.f, 0.f }, glm::radians(-90.f));

		auto const matrices = bump::camera_matrices(camera);

		m_frame_positions.clear();
		m_frame_positions.reserve(screen.size());

		m_frame_layers.clear();
		m_frame_layers.reserve(screen.size());

		m_frame_fg_colors.clear();
		m_frame_fg_colors.reserve(screen.size());
		
		m_frame_bg_colors.clear();
		m_frame_bg_colors.reserve(screen.size());

		for (auto y : bump::range(0, screen.extents().y))
		{
			for (auto x : bump::range(0, screen.extents().x))
			{
				auto const& cell = screen.at({ x, y });

				m_frame_positions.push_back(glm::vec2(x, y) * m_tile_size);
				m_frame_layers.push_back(static_cast<float>(cell.m_value));
				m_frame_fg_colors.push_back(cell.m_fg);
				m_frame_bg_colors.push_back(cell.m_bg);
			}
		}

		m_tile_renderable.render(renderer, matrices, *m_tile_texture, 
			m_frame_positions,
			m_frame_layers,
			m_frame_fg_colors,
			m_frame_bg_colors,
			m_tile_size);

	}
	
} // rog
