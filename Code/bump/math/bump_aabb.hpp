#pragma once

#include <bump_comparison.hpp>
#include <bump_math.hpp>
#include <bump_range.hpp>

namespace bump
{
	
	template<class T, glm::length_t D, glm::qualifier Q = glm::defaultp>
	struct aabb
	{
		using vec_type = glm::vec<D, T, Q>;

		vec_type m_origin;
		vec_type m_size;

		bool valid() const { return m_size.x >= 0 && m_size.y >= 0; }

		bool contains(vec_type const& point) const
		{
			return all_gte(point, m_origin) && all_lt(point, m_origin + m_size);
		}

	};

	using aabb2 = aabb<float, 2>;
	using iaabb2 = aabb<int, 2>;

	using aabb3 = aabb<float, 3>;
	using iaabb3 = aabb<int, 3>;

} // bump
