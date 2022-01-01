#pragma once

#include "bump_font_asset.hpp"
#include "bump_sdl_mixer_chunk.hpp"
#include "bump_sdl_mixer_music.hpp"
#include "bump_gl_shader.hpp"
#include "bump_mbp_model.hpp"
#include "bump_gl_texture.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace bump
{

	class app;

	struct font_metadata
	{
		std::string m_name;
		std::string m_filename;
		std::uint32_t m_size_pixels_per_em;
	};

	struct sound_metadata
	{
		std::string m_name;
		std::string m_filename;
	};

	struct music_metadata
	{
		std::string m_name;
		std::string m_filename;
	};

	struct shader_metadata
	{
		std::string m_name;
		std::vector<std::string> m_filenames;
	};

	struct model_metadata
	{
		std::string m_name;
		std::string m_filename;
	};

	struct texture_parameters_metadata
	{
		GLenum m_internal_format;
		GLenum m_data_format;
		GLenum m_min_filter = GL_NEAREST;
		GLenum m_mag_filter = GL_NEAREST;
		GLenum m_wrap_mode = GL_CLAMP_TO_EDGE;
		GLfloat m_anisotropy = 1.f;
		bool m_generate_mipmaps = false;
	};

	struct texture_2d_array_metadata
	{
		std::string m_name;
		std::string m_filename;
		std::uint32_t m_num_layers;
		texture_parameters_metadata m_parameters;
	};

	struct texture_cubemap_metadata
	{
		std::string m_name;
		std::array<std::string, 6u> m_filenames; // pos_x, neg_x, pos_y, neg_y, pos_z, neg_z
		texture_parameters_metadata m_parameters;
	};

	struct asset_metadata
	{
		std::vector<font_metadata> m_fonts;
		std::vector<sound_metadata> m_sounds;
		std::vector<music_metadata> m_music;
		std::vector<shader_metadata> m_shaders;
		std::vector<model_metadata> m_models;
		std::vector<texture_2d_array_metadata> m_texture_2d_arrays;
		std::vector<texture_cubemap_metadata> m_texture_cubemaps;
	};

	class assets
	{
	public:

		std::unordered_map<std::string, font::font_asset> m_fonts;
		std::unordered_map<std::string, sdl::mixer_chunk> m_sounds;
		std::unordered_map<std::string, sdl::mixer_music> m_music;
		std::unordered_map<std::string, gl::shader_program> m_shaders;
		std::unordered_map<std::string, mbp_model> m_models;
		std::unordered_map<std::string, gl::texture_2d_array> m_texture_2d_arrays;
		std::unordered_map<std::string, gl::texture_cubemap> m_texture_cubemaps;
	};

	assets load_assets(app& app, asset_metadata const& metadata);
	
} // bump