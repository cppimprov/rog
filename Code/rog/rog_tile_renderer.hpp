#pragma once

#include <bump_app.hpp>
#include <bump_camera.hpp>
#include <bump_gl.hpp>

#include <glm/glm.hpp>

#include <vector>

namespace rog
{

	class screen_buffer;
	
	class tile_renderer
	{
	public:

		explicit tile_renderer(bump::app& app, glm::vec2 tile_size);
	
		void render(bump::gl::renderer& renderer, glm::vec2 window_size, screen_buffer const& screen);

	private:

		class tile_renderable
		{
		public:

			explicit tile_renderable(bump::gl::shader_program const& shader);

			tile_renderable(tile_renderable const&) = delete;
			tile_renderable& operator=(tile_renderable const&) = delete;

			tile_renderable(tile_renderable &&) = default;
			tile_renderable& operator=(tile_renderable &&) = default;

			void render(bump::gl::renderer& renderer, 
				bump::camera_matrices const& matrices, 
				bump::gl::texture_2d_array const& texture, 
				std::vector<glm::vec2> const& positions,
				std::vector<float> const& layers,
				std::vector<glm::vec3> const& fg_colors,
				std::vector<glm::vec3> const& bg_colors,
				glm::vec2 tile_size);

		private:

			bump::gl::shader_program const* m_shader;
			GLint m_in_VertexPosition;
			GLint m_in_TilePosition;
			GLint m_in_TileLayer;
			GLint m_in_TileFGColor;
			GLint m_in_TileBGColor;
			GLint m_u_TileSize;
			GLint m_u_TileTexture;
			GLint m_u_MVP;

			bump::gl::buffer m_vertex_buffer;
			bump::gl::buffer m_tile_positions_buffer;
			bump::gl::buffer m_tile_layers_buffer;
			bump::gl::buffer m_tile_fg_colors_buffer;
			bump::gl::buffer m_tile_bg_colors_buffer;
			bump::gl::vertex_array m_vertex_array;
		};

		glm::vec2 m_tile_size;
		bump::gl::texture_2d_array* m_tile_texture;
		tile_renderable m_tile_renderable;

		std::vector<glm::vec2> m_frame_positions;
		std::vector<float> m_frame_layers;
		std::vector<glm::vec3> m_frame_fg_colors;
		std::vector<glm::vec3> m_frame_bg_colors;
	};

} // rog
