#include "bump_ui_renderer.hpp"

namespace bump::ui
{

	renderer::renderer(
		gl::shader_program const& rect_shader,
		gl::shader_program const& textured_rect_shader,
		gl::shader_program const& text_shader):
		m_rect
		{
			.m_shader = &rect_shader,
			.m_in_VertexPosition = rect_shader.get_attribute_location("in_VertexPosition"),
			.m_u_Position = rect_shader.get_uniform_location("u_Position"),
			.m_u_Size = rect_shader.get_uniform_location("u_Size"),
			.m_u_Color = rect_shader.get_uniform_location("u_Color"),
			.m_u_MVP = rect_shader.get_uniform_location("u_MVP")
		},
		m_textured_rect
		{
			.m_shader = &textured_rect_shader,
			.m_in_VertexPosition = textured_rect_shader.get_attribute_location("in_VertexPosition"),
			.m_u_Position = textured_rect_shader.get_uniform_location("u_Position"),
			.m_u_Size = textured_rect_shader.get_uniform_location("u_Size"),
			.m_u_Texture = textured_rect_shader.get_uniform_location("u_Texture"),
			.m_u_MVP = textured_rect_shader.get_uniform_location("u_MVP")
		},
		m_text
		{
			.m_shader = &text_shader,
			.m_in_VertexPosition = text_shader.get_attribute_location("in_VertexPosition"),
			.m_u_Position = text_shader.get_uniform_location("u_Position"),
			.m_u_Size = text_shader.get_uniform_location("u_Size"),
			.m_u_Offset = text_shader.get_uniform_location("u_Offset"),
			.m_u_Color = text_shader.get_uniform_location("u_Color"),
			.m_u_Texture = text_shader.get_uniform_location("u_Texture"),
			.m_u_MVP = text_shader.get_uniform_location("u_MVP")
		}
	{
		auto const vertices = { 0.f, 0.f,  0.f, 1.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f, 1.f, 0.f  };

		m_rect.m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_rect.m_vertex_array.set_array_buffer(m_rect.m_in_VertexPosition, m_rect.m_vertex_buffer);
		
		m_textured_rect.m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_textured_rect.m_vertex_array.set_array_buffer(m_textured_rect.m_in_VertexPosition, m_textured_rect.m_vertex_buffer);

		m_text.m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_text.m_vertex_array.set_array_buffer(m_text.m_in_VertexPosition, m_text.m_vertex_buffer);
	}

	void renderer::draw_rect(gl::renderer& renderer, camera_matrices const& camera, vec position, vec size, glm::vec4 color) const
	{
		renderer.set_program(*m_rect.m_shader);
		renderer.set_uniform_2f(m_rect.m_u_Position, glm::vec2(position));
		renderer.set_uniform_2f(m_rect.m_u_Size, glm::vec2(size));
		renderer.set_uniform_4f(m_rect.m_u_Color, color);
		renderer.set_uniform_4x4f(m_rect.m_u_MVP, camera.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_rect.m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_rect.m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

	void renderer::draw_textured_rect(gl::renderer &renderer, camera_matrices const &camera, vec position, vec size, gl::texture_2d const &texture) const
	{
		renderer.set_program(*m_textured_rect.m_shader);
		renderer.set_texture_2d(0u, texture);
		renderer.set_uniform_2f(m_textured_rect.m_u_Position, glm::vec2(position));
		renderer.set_uniform_2f(m_textured_rect.m_u_Size, glm::vec2(size));
		renderer.set_uniform_1i(m_textured_rect.m_u_Texture, 0);
		renderer.set_uniform_4x4f(m_textured_rect.m_u_MVP, camera.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_textured_rect.m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_textured_rect.m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_texture_2d(0u);
		renderer.clear_program();
	}

	void renderer::draw_text(gl::renderer &renderer, camera_matrices const &camera, vec position, text_texture const &text, vec::value_type line_height, glm::vec4 color) const
	{
		auto const offset = vec{ text.m_pos.x, (line_height - 1) - (text.m_pos.y + text.m_texture.get_size().y) };

		renderer.set_program(*m_text.m_shader);
		renderer.set_uniform_2f(m_text.m_u_Position, glm::vec2(position));
		renderer.set_uniform_2f(m_text.m_u_Size, glm::vec2(text.m_texture.get_size()));
		renderer.set_uniform_2f(m_text.m_u_Offset, glm::vec2(offset));
		renderer.set_uniform_4f(m_text.m_u_Color, color);
		renderer.set_uniform_1i(m_text.m_u_Texture, 0);
		renderer.set_uniform_4x4f(m_text.m_u_MVP, camera.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_texture_2d(0u, text.m_texture);
		renderer.set_vertex_array(m_text.m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_text.m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_texture_2d(0u);
		renderer.clear_program();
	}

} // bump::ui
