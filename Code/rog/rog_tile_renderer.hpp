#pragma once

#include <bump_camera.hpp>
#include <bump_gl.hpp>
#include <bump_math.hpp>

#include <vector>

namespace rog
{

	struct tile_instance_data
	{
		std::vector<glm::vec2> positions;
		std::vector<float> layers;
		std::vector<glm::vec3> fg_colors;
		std::vector<glm::vec3> bg_colors;

		void clear() { positions.clear(); layers.clear(); fg_colors.clear(); bg_colors.clear(); }
		void reserve(std::size_t count) { positions.reserve(count); layers.reserve(count); fg_colors.reserve(count); bg_colors.reserve(count); }
	};

	class tile_renderer
	{
	public:

		explicit tile_renderer(bump::gl::shader_program const& shader, bump::gl::texture_2d_array const& texture, glm::vec2 tile_size);

		void render(
			bump::gl::renderer& renderer, 
			bump::camera_matrices const& matrices,
			tile_instance_data const& instances);

	private:

		bump::gl::shader_program const* m_shader;
		bump::gl::texture_2d_array const* m_texture;

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
		
		glm::vec2 m_tile_size;
	};

} // rog
