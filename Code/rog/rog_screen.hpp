#pragma once

#include "rog_colors.hpp"

#include <bump_aabb.hpp>
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
		glm::vec3 m_fg = glm::vec3(1.f);
		glm::vec3 m_bg = glm::vec3(0.f);
		glm::vec3 m_border = glm::vec3(0.f);
		std::uint32_t m_border_width = 0;
	};

	auto static constexpr screen_cell_blank = screen_cell{ ' ', colors::white, colors::black, colors::black, 0 };
	auto static constexpr screen_cell_debug = screen_cell{ '#', colors::violet, colors::dark_red, colors::violet, 1 };

	struct screen_buffer
	{
		void fill(screen_cell const& cell);
		void fill_rect(glm::ivec2 origin, glm::ivec2 size, screen_cell const& cell);

		glm::ivec2 size() const { return glm::ivec2(m_data.extents()); }
		void resize(glm::ivec2 size, screen_cell const& cell);

		bool in_bounds(glm::ivec2 pos) const { return bump::iaabb2{ { 0, 0 }, m_data.extents() }.contains(pos); }

		bump::grid2<screen_cell, glm::ivec2> m_data;
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

	class tile_renderable
	{
	public:

		explicit tile_renderable(bump::gl::shader_program const& shader, bump::gl::texture_2d_array const& texture);

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

	struct tile_border_instance_data
	{
		std::vector<glm::vec2> positions;
		std::vector<float> widths;
		std::vector<glm::vec3> colors;

		void clear() { positions.clear(); widths.clear(); colors.clear(); }
		void reserve(std::size_t count) { positions.reserve(count); widths.reserve(count); colors.reserve(count); }
	};

	class tile_border_renderable
	{
	public:

		explicit tile_border_renderable(bump::gl::shader_program const& shader);

		void render(
			bump::gl::renderer& renderer,
			bump::camera_matrices const& matrices,
			tile_border_instance_data const& instances,
			glm::vec2 tile_size_px);

	private:

		bump::gl::shader_program const* m_shader;

		GLint m_in_VertexPosition;
		GLint m_in_VertexLerp;
		GLint m_in_BorderPosition;
		GLint m_in_BorderWidth;
		GLint m_in_BorderColor;
		GLint m_u_TileSize;
		GLint m_u_MVP;

		bump::gl::buffer m_vertex_buffer;
		bump::gl::buffer m_vertex_lerp_buffer;
		bump::gl::buffer m_border_positions_buffer;
		bump::gl::buffer m_border_widths_buffer;
		bump::gl::buffer m_border_colors_buffer;
		bump::gl::vertex_array m_vertex_array;
	};

	/*
	 * Coordinate systems:
	 * 
	 * px - screen: (0, 0) is bottom-left (OpenGL convention)
	 * sb - screenbuffer: (0, 0) is top-left
	 * pn - panel: (0, 0) is top-left (local area inside screenbuffer)
	 * lv - level: (0, 0) is top-left (map coords)
	 * 
	 */

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

	class screen
	{
	public:

		explicit screen(
			bump::gl::shader_program const& tile_shader, bump::gl::texture_2d_array const& tile_texture,
			bump::gl::shader_program const& border_shader,
			glm::ivec2 window_size_px, glm::ivec2 tile_size_px);

		glm::ivec2 size() const { return m_buffer.m_data.extents(); }
		glm::ivec2 tile_size() const { return m_tile_size_px; }

		void resize(glm::ivec2 window_size_px, glm::ivec2 tile_size_px);

		void render(bump::gl::renderer& renderer);

		glm::ivec2 px_to_sb(glm::ivec2 px) const { return screen_px_to_buffer_cell(px, m_sb_area_px.m_origin, tile_size(), size()); }
		glm::ivec2 sb_to_px(glm::ivec2 sb) const { return buffer_cell_to_screen_px(sb, m_sb_area_px.m_origin, tile_size(), size()); }
		glm::ivec2 sb_to_pn(glm::ivec2 sb, glm::ivec2 panel_origin_sb) const { return buffer_cell_to_panel_cell(sb, panel_origin_sb); }
		glm::ivec2 pn_to_sb(glm::ivec2 pn, glm::ivec2 panel_origin_sb) const { return panel_cell_to_buffer_cell(pn, panel_origin_sb); }

		screen_buffer& buffer() { return m_buffer; }
		screen_buffer const& buffer() const { return m_buffer; }

	private:

		glm::ivec2 m_window_size_px;
		glm::ivec2 m_tile_size_px;
		bump::iaabb2 m_sb_area_px;

		screen_buffer m_buffer;
		tile_instance_data m_tile_instances;
		tile_renderable m_tile_renderable;
		tile_border_instance_data m_tile_border_instances;
		tile_border_renderable m_tile_border_renderable;
	};
	
	void draw_level(screen_buffer& sb, level const& level, bump::iaabb2 const& map_panel_sb);

} // rog
