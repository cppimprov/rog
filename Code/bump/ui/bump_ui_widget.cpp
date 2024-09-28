#include "bump_ui_widget.hpp"

namespace bump::ui
{

	quad::quad(gl::shader_program const& shader):
		m_shader(&shader),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_u_Position(shader.get_uniform_location("u_Position")),
		m_u_Size(shader.get_uniform_location("u_Size")),
		m_u_Color(shader.get_uniform_location("u_Color")),
		m_u_MVP(shader.get_uniform_location("u_MVP"))
	{
		//auto const vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
		auto const vertices = { 0.f, 0.f,  0.f, 1.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f, 1.f, 0.f  };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
	}

	void quad::render(bump::gl::renderer& renderer, bump::camera_matrices const& matrices)
	{
		renderer.set_program(*m_shader);
		renderer.set_uniform_2f(m_u_Position, glm::vec2(position));
		renderer.set_uniform_2f(m_u_Size, glm::vec2(size));
		renderer.set_uniform_4f(m_u_Color, color);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

} // bump::ui
