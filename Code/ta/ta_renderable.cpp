#include "ta_renderable.hpp"

namespace ta
{

	tile_renderable::tile_renderable(bump::gl::shader_program const &shader):
		m_shader(&shader),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_u_Size(shader.get_uniform_location("u_Size")),
		m_u_MVP(shader.get_uniform_location("u_MVP")),
		m_u_Texture(shader.get_uniform_location("u_Texture"))
	{
		auto const vertices = { -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, 0.5f,  -0.5f, -0.5f,  0.5f, 0.5f,  -0.5f, 0.5f, };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
	}

	void tile_renderable::render(
		bump::gl::renderer &renderer,
		bump::gl::texture_2d const &texture,
		bump::camera_matrices const &matrices,
		glm::mat4 model_matrix,
		glm::vec2 size) const
	{
		renderer.set_program(*m_shader);

		renderer.set_texture_2d(0, texture);

		renderer.set_uniform_2f(m_u_Size, size);
		renderer.set_uniform_1i(m_u_Texture, 0);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(model_matrix));

		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

	
	object_renderable::object_renderable(bump::gl::shader_program const &shader, bump::gl::texture_2d const &texture, bump::gl::texture_2d const &accent_texture):
		m_shader(&shader),
		m_texture(&texture),
		m_accent_texture(&accent_texture),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_u_Size(shader.get_uniform_location("u_Size")),
		m_u_AccentColor(shader.get_uniform_location("u_AccentColor")),
		m_u_MVP(shader.get_uniform_location("u_MVP")),
		m_u_Texture(shader.get_uniform_location("u_Texture")),
		m_u_AccentTexture(shader.get_uniform_location("u_AccentTexture"))
	{
		auto const vertices = { -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, 0.5f,  -0.5f, -0.5f,  0.5f, 0.5f,  -0.5f, 0.5f, };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
	}

	void object_renderable::render(
		bump::gl::renderer &renderer,
		bump::camera_matrices const &matrices,
		glm::mat4 model_matrix,
		glm::vec2 size,
		glm::vec3 accent_color) const
	{
		renderer.set_program(*m_shader);

		renderer.set_texture_2d(0, *m_texture);
		renderer.set_texture_2d(1, *m_accent_texture);

		renderer.set_uniform_2f(m_u_Size, size);
		renderer.set_uniform_3f(m_u_AccentColor, accent_color);
		renderer.set_uniform_1i(m_u_Texture, 0);
		renderer.set_uniform_1i(m_u_AccentTexture, 1);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(model_matrix));

		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

} // ta
