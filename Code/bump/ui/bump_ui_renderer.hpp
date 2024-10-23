#pragma once

#include "bump_gl.hpp"
#include "bump_camera.hpp"
#include "bump_render_text.hpp"
#include "bump_ui_vec.hpp"

namespace bump::ui
{

	class renderer
	{
	public:

		renderer(
			gl::shader_program const& rect_shader,
			gl::shader_program const& textured_rect_shader,
			gl::shader_program const& text_shader);

		void draw_rect(gl::renderer& renderer, camera_matrices const& camera,
			vec position, vec size, glm::vec4 color) const;

		void draw_textured_rect(gl::renderer& renderer, camera_matrices const& camera,
			vec position, vec size, gl::texture_2d const& texture) const;

		void draw_text(gl::renderer& renderer, camera_matrices const& camera,
			vec position, text_texture const& text, glm::vec4 color) const;

	private:

		struct rect
		{
			gl::shader_program const* m_shader;

			GLint m_in_VertexPosition;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Color;
			GLint m_u_MVP;
			
			gl::buffer m_vertex_buffer;
			gl::vertex_array m_vertex_array;
		}
		m_rect;

		struct textured_rect_uniforms
		{
			gl::shader_program const* m_shader;

			GLint m_in_VertexPosition;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Texture;
			GLint m_u_MVP;

			gl::buffer m_vertex_buffer;
			gl::vertex_array m_vertex_array;
		}
		m_textured_rect;
		
		struct text_uniforms
		{
			gl::shader_program const* m_shader;
			
			GLint m_in_VertexPosition;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Offset;
			GLint m_u_Color;
			GLint m_u_Texture;
			GLint m_u_MVP;

			gl::buffer m_vertex_buffer;
			gl::vertex_array m_vertex_array;
		}
		m_text;
	};

} // bump::ui
