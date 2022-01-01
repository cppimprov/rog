#include "rog_level_gen.hpp"

#include "rog_feature.hpp"

namespace rog
{
	
	namespace level_gen
	{
		
		void generate_level(bump::grid2<feature>& level, glm::size2 size)
		{
			level.clear();
			level.resize(size, features::grass);

			for (auto y : bump::range(1, size.y - 1))
			{
				for (auto x : bump::range(1, size.x - 1))
				{
					auto const n = glm::simplex(glm::vec2{ x, y });

					level.at({ x, y }) = (n > 0.0f) ? features::wall : features::floor;
				}
			}
			
			// ...
		}
		
	} // level_gen
	
} // rog
