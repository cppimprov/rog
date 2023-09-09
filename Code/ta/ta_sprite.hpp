#pragma once

#include <bump_camera.hpp>
#include <bump_gl.hpp>

namespace ta
{

	class tile_renderable
	{
	public:

		explicit tile_renderable(bump::gl::shader_program const& shader);

		tile_renderable(tile_renderable const&) = delete;
		tile_renderable& operator=(tile_renderable const&) = delete;

		tile_renderable(tile_renderable &&) = default;
		tile_renderable& operator=(tile_renderable &&) = default;

		void render(
			bump::gl::renderer& renderer,
			bump::gl::texture_2d const& texture,
			bump::camera_matrices const& matrices,
			glm::mat4 model_matrix,
			glm::vec2 size);

	private:

		bump::gl::shader_program const* m_shader;

		GLint m_in_VertexPosition;
		GLint m_u_Position;
		GLint m_u_Size;
		GLint m_u_MVP;
		GLint m_u_Texture;

		bump::gl::buffer m_vertex_buffer;
		bump::gl::vertex_array m_vertex_array;
	};
	
	class object_renderable
	{
	public:

		explicit object_renderable(bump::gl::shader_program const& shader, bump::gl::texture_2d const& texture, bump::gl::texture_2d const& accent_texture);

		object_renderable(object_renderable const&) = delete;
		object_renderable& operator=(object_renderable const&) = delete;

		object_renderable(object_renderable &&) = default;
		object_renderable& operator=(object_renderable &&) = default;

		void render(
			bump::gl::renderer& renderer,
			bump::camera_matrices const& matrices,
			glm::mat4 model_matrix,
			glm::vec2 size,
			glm::vec3 accent_color);

	private:

		bump::gl::shader_program const* m_shader;

		bump::gl::texture_2d const* m_texture;
		bump::gl::texture_2d const* m_accent_texture;

		GLint m_in_VertexPosition;
		GLint m_u_Position;
		GLint m_u_Size;
		GLint m_u_AccentColor;
		GLint m_u_MVP;
		GLint m_u_Texture;
		GLint m_u_AccentTexture;

		bump::gl::buffer m_vertex_buffer;
		bump::gl::vertex_array m_vertex_array;
	};

} // ta
