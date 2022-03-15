#pragma once

#include "bump_image.hpp"

#include <string>

namespace bump
{
	
	image<std::uint8_t> load_image_from_file(std::string const& file, bool flip = true);

	void write_png(std::string const& filename, image<std::uint8_t> const& image, bool flip = true);
	
} // bump
