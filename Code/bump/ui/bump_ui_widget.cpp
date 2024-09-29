#include "bump_ui_widget.hpp"

#include "bump_render_text.hpp"

#include <iostream>

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
	
	textured_quad::textured_quad(gl::shader_program const& shader, gl::texture_2d const& texture):
		m_shader(&shader),
		m_texture(&texture),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_u_Position(shader.get_uniform_location("u_Position")),
		m_u_Size(shader.get_uniform_location("u_Size")),
		m_u_Color(shader.get_uniform_location("u_Color")),
		m_u_Texture(shader.get_uniform_location("u_Texture")),
		m_u_MVP(shader.get_uniform_location("u_MVP"))
	{
		auto const vertices = { 0.f, 0.f,  0.f, 1.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f, 1.f, 0.f  };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);
	}

	void textured_quad::render(bump::gl::renderer& renderer, bump::camera_matrices const& matrices)
	{
		renderer.set_program(*m_shader);
		renderer.set_texture_2d(0u, *m_texture);
		renderer.set_uniform_2f(m_u_Position, glm::vec2(position));
		renderer.set_uniform_2f(m_u_Size, glm::vec2(size));
		renderer.set_uniform_4f(m_u_Color, color);
		renderer.set_uniform_1i(m_u_Texture, 0);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_texture_2d(0u);
		renderer.clear_program();
	}
	
	label::label(gl::shader_program const& shader, font::ft_context const& ft_context, font::font_asset const& font, std::string const& text):
		m_shader(&shader),
		m_ft_context(&ft_context),
		m_font(&font),
		m_text(text),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_u_Position(shader.get_uniform_location("u_Position")),
		m_u_Size(shader.get_uniform_location("u_Size")),
		m_u_Offset(shader.get_uniform_location("u_Offset")),
		m_u_Color(shader.get_uniform_location("u_Color")),
		m_u_Texture(shader.get_uniform_location("u_Texture")),
		m_u_MVP(shader.get_uniform_location("u_MVP"))
	{
		auto const vertices = { 0.f, 0.f,  0.f, 1.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f, 1.f, 0.f  };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);

		set_text(text);
	}

	void label::set_text(std::string const& text)
	{
		m_text = text;
		m_texture = render_text_to_gl_texture(*m_ft_context, *m_font, m_text);
	}

	void label::render(bump::gl::renderer& renderer, bump::camera_matrices const& matrices)
	{
		renderer.set_program(*m_shader);
		renderer.set_uniform_2f(m_u_Position, glm::vec2(position));
		renderer.set_uniform_2f(m_u_Size, glm::vec2(m_texture.m_texture.get_size()));
		renderer.set_uniform_2f(m_u_Offset, glm::vec2(m_texture.m_pos));
		renderer.set_uniform_4f(m_u_Color, color);
		renderer.set_uniform_1i(m_u_Texture, 0);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_texture_2d(0u, m_texture.m_texture);
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_texture_2d(0u);
		renderer.clear_program();
	}

} // bump::ui
