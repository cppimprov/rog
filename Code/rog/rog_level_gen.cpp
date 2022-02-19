#include "rog_level_gen.hpp"

#include "rog_feature.hpp"
#include "rog_random.hpp"

#include <bump_die.hpp>

#include <random>

namespace rog
{
	
	namespace level_gen
	{

		level generate_level(std::int32_t depth)
		{
			return level{ depth, bump::grid2<feature>({ 4, 4 }, features::floor) };
		}
		
	} // level_gen
	
} // rog
