#include "rog_level_gen.hpp"

#include "rog_feature.hpp"

namespace rog
{
	
	namespace level_gen
	{
		
		level generate_level(std::int32_t depth)
		{
			auto size = glm::size2(128);

			auto out = level();
			out.m_depth = depth;

			out.m_grid.resize(size, features::grass);

			for (auto y : bump::range(1, size.y - 1))
			{
				for (auto x : bump::range(1, size.x - 1))
				{
					auto const n = glm::simplex(glm::vec2{ x, y });

					out.m_grid.at({ x, y }) = (n > 0.0f) ? features::wall : features::floor;
				}
			}
			
			return out;
		}
		
	} // level_gen
	
} // rog
