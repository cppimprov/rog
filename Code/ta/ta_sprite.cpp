#include "ta_sprite.hpp"

namespace ta
{

	tank_renderable::tank_renderable(bump::gl::shader_program const &shader):
		m_shader(&shader),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_u_Position(shader.get_uniform_location("u_Position")),
		m_u_Size(shader.get_uniform_location("u_Size")),
		m_u_Color(shader.get_uniform_location("u_Color")),
		m_u_MVP(shader.get_uniform_location("u_MVP"))
	{
		// create vertex buffer
		auto const vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
	}

	void tank_renderable::render(
		bump::gl::renderer &renderer,
		bump::camera_matrices const &matrices,
		glm::vec2 position,
		glm::vec2 size,
		glm::vec3 color)
	{
		renderer.set_program(*m_shader);
		renderer.set_uniform_2f(m_u_Position, position);
		renderer.set_uniform_2f(m_u_Size, size);
		renderer.set_uniform_3f(m_u_Color, color);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

	bullet_renderable::bullet_renderable(bump::gl::shader_program const &shader):
		m_shader(&shader),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_u_Position(shader.get_uniform_location("u_Position")),
		m_u_Size(shader.get_uniform_location("u_Size")),
		m_u_Color(shader.get_uniform_location("u_Color")),
		m_u_MVP(shader.get_uniform_location("u_MVP"))
	{
		// create vertex buffer
		auto const vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
	}

	void bullet_renderable::render(
		bump::gl::renderer &renderer,
		bump::camera_matrices const &matrices,
		glm::vec2 position,
		glm::vec2 size,
		glm::vec3 color)
	{
		renderer.set_program(*m_shader);
		renderer.set_uniform_2f(m_u_Position, position);
		renderer.set_uniform_2f(m_u_Size, size);
		renderer.set_uniform_3f(m_u_Color, color);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

} // ta
