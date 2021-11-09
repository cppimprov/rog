#include "rog_screen.hpp"

#include <algorithm>

namespace rog
{

	namespace screen
	{
		
		void fill(buffer& screen, cell const& cell)
		{
			fill_rect(screen, { 0, 0 }, screen.extents(), cell);
		}

		void fill_rect(buffer& screen, glm::size2 origin, glm::size2 size, cell const& cell)
		{
			auto const begin = glm::min(origin, screen.extents());
			auto const end = glm::min(origin + size, screen.extents());

			for (auto y : bump::range(begin.y, end.y))
				for (auto x : bump::range(begin.x, end.x))
					screen.at({ x, y }) = cell;
		}

		void print_char(buffer& screen, glm::size2 origin, cell const& cell)
		{
			if (glm::any(glm::greaterThanEqual(origin, screen.extents())))
				return;
			
			screen.at(origin) = cell;
		}

		void print_string(buffer& screen, glm::size2 origin, std::string const& s, glm::vec3 fg, glm::vec3 bg)
		{
			if (s.empty())
				return;

			if (glm::any(glm::greaterThanEqual(origin, screen.extents())))
				return;

			auto length = std::min(s.size(), screen.extents().x - origin.x);

			for (auto i = std::size_t{ 0 }; i != length; ++i)
				screen.at({ origin.x + i, origin.y }) = { static_cast<std::uint8_t>(s[i]), fg, bg };
		}

		void resize(buffer& screen, glm::ivec2 window_size, glm::ivec2 tile_size, cell const& cell)
		{
			auto const num_tiles = window_size / tile_size;
			screen.resize(glm::size2(num_tiles), cell);
		}
		
	} // screen
	
} // rog
