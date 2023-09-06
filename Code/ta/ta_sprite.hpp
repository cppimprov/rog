#pragma once

#include <bump_camera.hpp>
#include <bump_gl.hpp>

namespace ta
{

	class sprite
	{
	public:

		explicit sprite(bump::gl::shader_program const& shader);

		sprite(sprite const&) = delete;
		sprite& operator=(sprite const&) = delete;

		sprite(sprite &&) = default;
		sprite& operator=(sprite &&) = default;

		void render(bump::gl::renderer& renderer,
			bump::camera_matrices const& matrices,
			glm::vec2 position,
			glm::vec2 size,
			glm::vec3 accent_color);

	private:

			bump::gl::shader_program const* m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_AccentColor;
			GLint m_u_MVP;

			bump::gl::buffer m_vertex_buffer;
			bump::gl::vertex_array m_vertex_array;
	};

} // ta
