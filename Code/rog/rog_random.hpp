#pragma once

#include <random>

namespace rog
{
	
	namespace random
	{
		
		using rng_t = std::mt19937_64;

		rng_t seed_rng();

		template<class T>
		T rand_01(rng_t& rng)
		{
			using dist_t = std::uniform_real_distribution<T>;
			return dist_t(T{0}, T{1})(rng);
		}
		
	} // random
	
} // rog
