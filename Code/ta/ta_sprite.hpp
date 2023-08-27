#pragma once

#include <bump_camera.hpp>
#include <bump_gl.hpp>

namespace ta
{
	class tank_renderable
	{
	public:

		explicit tank_renderable(bump::gl::shader_program const& shader);

		tank_renderable(tank_renderable const&) = delete;
		tank_renderable& operator=(tank_renderable const&) = delete;

		tank_renderable(tank_renderable &&) = default;
		tank_renderable& operator=(tank_renderable &&) = default;

		void render(bump::gl::renderer& renderer,
			bump::camera_matrices const& matrices,
			glm::vec2 position,
			glm::vec2 size,
			glm::vec3 color);

	private:

			bump::gl::shader_program const* m_shader;
			GLint m_in_VertexPosition;
			GLint m_u_Position;
			GLint m_u_Size;
			GLint m_u_Color;
			GLint m_u_MVP;

			bump::gl::buffer m_vertex_buffer;
			bump::gl::vertex_array m_vertex_array;
	};

	// todo:
	// bullet_renderable
	// powerup_renderable

} // ta
