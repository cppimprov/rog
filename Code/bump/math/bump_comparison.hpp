#pragma once

#include <bump_math.hpp>

namespace bump
{

	template<class T, glm::length_t D, glm::qualifier Q = glm::defaultp>
	bool all_eq(glm::vec<D, T, Q> const& a, glm::vec<D, T, Q> const& b)
	{
		for (auto i = glm::length_t{ 0 }; i != D; ++i)
			if (!(a[i] == b[i])) return false;
		return true;
	}

	template<class T, glm::length_t D, glm::qualifier Q = glm::defaultp>
	bool all_ne(glm::vec<D, T, Q> const& a, glm::vec<D, T, Q> const& b)
	{
		for (auto i = glm::length_t{ 0 }; i != D; ++i)
			if (!(a[i] != b[i])) return false;
		return true;
	}

	template<class T, glm::length_t D, glm::qualifier Q = glm::defaultp>
	bool all_lt(glm::vec<D, T, Q> const& a, glm::vec<D, T, Q> const& b)
	{
		for (auto i = glm::length_t{ 0 }; i != D; ++i)
			if (!(a[i] < b[i])) return false;
		return true;
	}

	template<class T, glm::length_t D, glm::qualifier Q = glm::defaultp>
	bool all_gt(glm::vec<D, T, Q> const& a, glm::vec<D, T, Q> const& b)
	{
		for (auto i = glm::length_t{ 0 }; i != D; ++i)
			if (!(a[i] > b[i])) return false;
		return true;
	}

	template<class T, glm::length_t D, glm::qualifier Q = glm::defaultp>
	bool all_lte(glm::vec<D, T, Q> const& a, glm::vec<D, T, Q> const& b)
	{
		for (auto i = glm::length_t{ 0 }; i != D; ++i)
			if (!(a[i] <= b[i])) return false;
		return true;
	}

	template<class T, glm::length_t D, glm::qualifier Q = glm::defaultp>
	bool all_gte(glm::vec<D, T, Q> const& a, glm::vec<D, T, Q> const& b)
	{
		for (auto i = glm::length_t{ 0 }; i != D; ++i)
			if (!(a[i] >= b[i])) return false;
		return true;
	}

} // bump
