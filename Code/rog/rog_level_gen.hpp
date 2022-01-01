#pragma once

#include <bump_grid.hpp>
#include <bump_math.hpp>

namespace rog
{

	struct feature;
	
	namespace level_gen
	{
		
		void generate_level(bump::grid2<feature>& level, glm::size2 size);
		
	} // level_gen
	
} // rog
