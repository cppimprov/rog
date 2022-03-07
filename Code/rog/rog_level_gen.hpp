#pragma once

#include "rog_level.hpp"
#include "rog_random.hpp"

#include <bump_grid.hpp>
#include <bump_math.hpp>

namespace rog
{

	namespace level_gen
	{
		
		level generate_level(std::int32_t depth, random::rng_t& rng);
		
	} // level_gen
	
} // rog
