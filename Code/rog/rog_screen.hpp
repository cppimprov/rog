#pragma once

#include "rog_colors.hpp"

#include <bump_camera.hpp>
#include <bump_gl.hpp>
#include <bump_grid.hpp>
#include <bump_math.hpp>

#include <cstdint>

namespace bump { class app; }
namespace rog { struct level; }

namespace rog
{

	struct screen_cell
	{
		std::uint8_t m_value = ' ';
		glm::vec3 m_fg = glm::vec3(0.f);
		glm::vec3 m_bg = glm::vec3(0.f);
	};

	struct screen_buffer
	{
		void fill(screen_cell const& cell);
		void fill_rect(glm::ivec2 origin, glm::ivec2 size, screen_cell const& cell);

		glm::ivec2 size() const { return glm::ivec2(m_data.extents()); }
		void resize(glm::ivec2 size, screen_cell const& cell);

		bump::grid2<screen_cell> m_data;
	};
	
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

		explicit tile_renderer(bump::gl::shader_program const& shader, bump::gl::texture_2d_array const& texture);

		void render(
			bump::gl::renderer& renderer, 
			bump::camera_matrices const& matrices,
			tile_instance_data const& instances,
			glm::vec2 tile_size_px);

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
	};

	/*
	 * Coordinate systems:
	 *
	 * window: (0, 0) is top-left (SDL convention)
	 * screen: (0, 0) is bottom-left (OpenGL convention)
	 * screenbuffer: (0, 0) is top-left
	 * level: (0, 0) is top-left
	 * 
	 */

	inline glm::ivec2 window_px_to_screen_px(glm::ivec2 px, glm::ivec2 window_size_px)
	{
		return { px.x, (window_size_px.y - 1) - px.y };
	}
	
	inline glm::ivec2 screen_px_to_window_px(glm::ivec2 px, glm::ivec2 window_size_px)
	{
		return { px.x, (window_size_px.y - 1) - px.y };
	}

	inline glm::ivec2 screen_px_to_buffer_cell(glm::ivec2 px, glm::ivec2 sb_origin_px, glm::ivec2 tile_size_px, glm::ivec2 sb_size_sb)
	{
		auto const cells = (px - sb_origin_px) / tile_size_px;
		return { cells.x, (sb_size_sb.y - 1) - cells.y };
	}

	inline glm::ivec2 buffer_cell_to_screen_px(glm::ivec2 sb, glm::ivec2 sb_origin_px, glm::ivec2 tile_size_px, glm::ivec2 sb_size_sb)
	{
		auto const cells = glm::ivec2(sb.x, (sb_size_sb.y - 1) - sb.y);
		return cells * tile_size_px + sb_origin_px;
	}

	inline glm::ivec2 buffer_cell_to_panel_cell(glm::ivec2 sb, glm::ivec2 pn_origin_sb)
	{
		return sb - pn_origin_sb;
	}

	inline glm::ivec2 panel_cell_to_buffer_cell(glm::ivec2 pn, glm::ivec2 pn_origin_sb)
	{
		return pn + pn_origin_sb;
	}

	inline glm::ivec2 panel_cell_to_map_coords(glm::ivec2 pn, glm::ivec2 pn_origin_lv)
	{
		return pn + pn_origin_lv;
	}

	inline glm::ivec2 map_coords_to_panel_cell(glm::ivec2 lv, glm::ivec2 pn_origin_lv)
	{
		return lv - pn_origin_lv;
	}

	// temp:
	glm::ivec2 get_map_panel_origin(glm::ivec2 level_size, glm::ivec2 panel_size, glm::ivec2 focus);

	class screen
	{
	public:

		explicit screen(bump::gl::shader_program const& shader, bump::gl::texture_2d_array const& texture, glm::ivec2 window_size_px, glm::ivec2 tile_size_px);

		glm::ivec2 size() const { return glm::ivec2(m_buffer.m_data.extents()); }

		void resize(glm::ivec2 window_size_px, glm::ivec2 tile_size_px);

		void draw(level& level);
		void render(bump::gl::renderer& renderer);

	private:

		glm::ivec2 m_window_size_px;
		glm::ivec2 m_tile_size_px;
		glm::ivec2 m_sb_origin_px;
		glm::ivec2 m_sb_size_px;

		screen_buffer m_buffer;
		tile_instance_data m_tile_instances;
		tile_renderer m_tile_renderer;

		static constexpr screen_cell debug_cell = { '#', colors::violet, colors::dark_red };
	};

} // rog
