#include "rog_tile_renderer.hpp"

namespace rog
{
	
	tile_renderer::tile_renderer(bump::gl::shader_program const& shader, bump::gl::texture_2d_array const& texture, glm::vec2 tile_size):
		m_shader(&shader),
		m_texture(&texture),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_in_TilePosition(shader.get_attribute_location("in_TilePosition")),
		m_in_TileLayer(shader.get_attribute_location("in_TileLayer")),
		m_in_TileFGColor(shader.get_attribute_location("in_TileFGColor")),
		m_in_TileBGColor(shader.get_attribute_location("in_TileBGColor")),
		m_u_TileSize(shader.get_uniform_location("u_TileSize")),
		m_u_TileTexture(shader.get_uniform_location("u_TileTexture")),
		m_u_MVP(shader.get_uniform_location("u_MVP")),
		m_tile_size(tile_size)
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
		tile_instance_data const& instances)
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
		renderer.set_uniform_2f(m_u_TileSize, m_tile_size);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count(), instance_count);

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

} // rog
