#pragma once

#include <bump_math.hpp>

#include <random>
#include <type_traits>

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

		namespace impl
		{
			template<class T, class = void> struct rand_range_dist_t;
			template<class T> struct rand_range_dist_t<T, std::enable_if_t<std::is_integral_v<T>>> { using type = std::uniform_int_distribution<T>; };
			template<class T> struct rand_range_dist_t<T, std::enable_if_t<std::is_floating_point_v<T>>> { using type = std::uniform_real_distribution<T>; };
			
		} // impl

		template<class T, class = std::enable_if_t<std::is_arithmetic_v<T>>>
		T rand_range(rng_t& rng, T min, T max)
		{
			using dist_t = impl::rand_range_dist_t<T>::type;
			return dist_t(min, max)(rng);
		}

		template<glm::length_t S, class T, glm::qualifier Q, class = std::enable_if_t<std::is_arithmetic_v<T>> >
		glm::vec<S, T, Q> rand_range(rng_t& rng, glm::vec<S, T, Q> min, glm::vec<S, T, Q> max)
		{
			auto out = glm::vec<S, T, Q>();
			for (auto i = glm::length_t{ 0 }; i != S; ++i)
				out[i] = rand_range(rng, min[i], max[i]);
			
			return out;
		}
		
	} // random
	
} // rog
