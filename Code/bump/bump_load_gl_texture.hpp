#pragma once

#include "bump_gl_texture.hpp"

#include <array>
#include <string>

namespace bump
{
	
	struct texture_parameters_metadata;

	gl::texture_2d_array load_gl_texture_2d_array_from_file(std::string const& file, std::uint32_t num_layers, texture_parameters_metadata const& parameters);
	gl::texture_cubemap load_gl_cubemap_texture_from_files(std::array<std::string, 6> const& files, texture_parameters_metadata const& parameters);
	
} // bump
