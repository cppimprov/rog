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

	label_button::label_button(gl::shader_program const& shader, font::ft_context const& ft_context, font::font_asset const& font, std::string const& text):
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
		m_u_MVP(shader.get_uniform_location("u_MVP")),
		m_hovered(false),
		m_pressed(false)
	{
		auto const vertices = { 0.f, 0.f,  0.f, 1.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f, 1.f, 0.f  };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);

		set_text(text);
	}

	void label_button::set_text(std::string const& text)
	{
		m_text = text;
		m_texture = render_text_to_gl_texture(*m_ft_context, *m_font, m_text);
	}

	void label_button::render(bump::gl::renderer& renderer, bump::camera_matrices const& matrices)
	{
		renderer.set_program(*m_shader);
		renderer.set_uniform_2f(m_u_Position, glm::vec2(position));
		renderer.set_uniform_2f(m_u_Size, glm::vec2(m_texture.m_texture.get_size()));
		renderer.set_uniform_2f(m_u_Offset, glm::vec2(m_texture.m_pos));
		renderer.set_uniform_4f(m_u_Color, m_pressed ? press_color : m_hovered ? hover_color : inactive_color);
		renderer.set_uniform_1i(m_u_Texture, 0);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_projection_matrix(glm::identity<glm::mat4>()));
		renderer.set_texture_2d(0u, m_texture.m_texture);
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count());

		renderer.clear_vertex_array();
		renderer.clear_texture_2d(0u);
		renderer.clear_program();
	}
	
	text_field::text_field(gl::shader_program const& shader, font::ft_context const& ft_context, font::font_asset const& font, std::string const& text):
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
		m_u_MVP(shader.get_uniform_location("u_MVP")),
		m_hovered(false),
		m_pressed(false),
		m_focused(false),
		m_max_length(255),
		m_caret(0),
		m_selection(0)
	{
		auto const vertices = { 0.f, 0.f,  0.f, 1.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f, 1.f, 0.f  };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);

		insert_text(text);
	}

	void text_field::set_text(std::string const& text)
	{
		m_text = text;

		if (m_text.size() > m_max_length)
			m_text.resize(m_max_length);

		m_caret = std::min(m_caret, m_text.size());
		m_selection = std::min(m_selection, m_text.size());

		redraw_text();
	}

	void text_field::set_max_length(std::size_t length)
	{
		m_max_length = length;

		set_text(m_text);
	}

	void text_field::set_caret(std::size_t pos, bool select)
	{
		m_caret = std::min(pos, m_text.size());

		if (!select)
			m_selection = m_caret;
	}

	void text_field::input(input::input_event const& event)
	{
		namespace ie = input::input_events;

		if (std::holds_alternative<ie::mouse_motion>(event))
		{
			auto const& mm = std::get<ie::mouse_motion>(event);
			m_hovered = aabb{ position, size }.contains(mm.m_inv_y_position);
			m_pressed = m_pressed && m_hovered;
		}

		if (std::holds_alternative<ie::mouse_button>(event))
		{
			auto const& mb = std::get<ie::mouse_button>(event);
			
			if (mb.m_button == input::mouse_button::LEFT)
			{
				if (mb.m_value)
				{
					if (m_hovered)
					{
						m_pressed = true;
						m_focused = true;
					}
					else
					{
						m_pressed = false;
						m_focused = false;
					}
				}
				else
				{
					m_pressed = false;
				}
			}
		}

		if (m_focused)
		{
			if (std::holds_alternative<ie::typing>(event))
			{
				auto const& t = std::get<ie::typing>(event);

				insert_text(t.m_text.data());
			}

			if (std::holds_alternative<ie::keyboard_key>(event))
			{
				auto const& k = std::get<ie::keyboard_key>(event);

				using kt = bump::input::keyboard_key;

				if (k.m_key == kt::ESCAPE) { m_focused = false; }
				else if (k.m_key == kt::BACKSPACE) { if (!m_text.empty()) { m_text.pop_back(); set_text(m_text); } }
				// ...
			}
		}
	}

	void text_field::render(bump::gl::renderer& renderer, bump::camera_matrices const& matrices)
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
	
	void text_field::redraw_text()
	{
		m_texture = render_text_to_gl_texture(*m_ft_context, *m_font, m_text);
	}
	
	void text_field::insert_text(std::string const& text)
	{
		// erase any selected text
		if (selection_size() != 0)
		{
			m_text.erase(selection_start(), selection_size());
			set_caret(selection_start(), false);
		}

		// calculate available space
		auto const space = m_max_length - m_text.size();
		auto const len = std::min(space, text.size());

		// insert as much text as we can in the available space
		m_text.insert(m_caret, std::string_view(text.begin(), text.begin() + len));

		// move the caret to the end of the inserted text
		set_caret(m_caret + len, false);

		// update texture
		redraw_text();
	}

} // bump::ui
