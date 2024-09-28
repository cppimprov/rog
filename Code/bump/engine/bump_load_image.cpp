#include "bump_load_image.hpp"

#include "bump_die.hpp"
#include "bump_log.hpp"
#include "bump_narrow_cast.hpp"

#include <stb_image.h>
#include <stb_image_write.h>

namespace bump
{
	
	image<std::uint8_t> load_image_from_file(std::string const& file, bool flip)
	{
		stbi_set_flip_vertically_on_load(flip);

		auto width = 0;
		auto height = 0;
		auto channels = 0;
		auto pixels = stbi_load(file.c_str(), &width, &height, &channels, 0);

		if (!pixels)
		{
			log_error("stbi_load() failed: " + std::string(stbi_failure_reason()));
			die();
		}

		auto out = image<std::uint8_t>(channels, { width, height });
		std::copy_n(pixels, width * height * channels, out.data());

		stbi_image_free(pixels);

		return out;
	}

	void write_png(std::string const& filename, image<std::uint8_t> const& image, bool flip)
	{
		stbi_flip_vertically_on_write(flip);

		auto width = narrow_cast<int>(image.size().x);
		auto height = narrow_cast<int>(image.size().y);
		auto channels = narrow_cast<int>(image.channels());
		auto result = stbi_write_png(filename.c_str(), width, height, channels, image.data(), 0);

		if (!result)
		{
			log_error("stbi_write_png() failed to write image: " + filename);
			die();
		}
	}
	
} // bump
