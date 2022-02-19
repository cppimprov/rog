#include "rog_random.hpp"

#include <algorithm>
#include <array>

namespace rog
{
	
	namespace random
	{
		
		rng_t seed_rng()
		{
			auto rd = std::random_device();
			auto seq = std::array<std::seed_seq::result_type, rng_t::state_size>();
			std::generate_n(seq.begin(), rng_t::state_size, std::ref(rd));

			auto seed = std::seed_seq(seq.begin(), seq.end());

			return rng_t(seed);
		}
		
	} // random
	
} // rog
