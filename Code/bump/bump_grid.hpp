#pragma once

#include "bump_die.hpp"
#include "bump_math.hpp"
#include "bump_range.hpp"

#include <cstdlib>
#include <initializer_list>
#include <vector>

namespace bump
{
	
	namespace grid_detail
	{

		template<std::size_t D, glm::qualifier Q>
		constexpr std::size_t get_data_size(glm::vec<D, std::size_t, Q> extents)
		{
			auto r = std::size_t{ 1 };

			for (auto i : range(0, D))
				r *= extents[i];
			
			return r;
		}

		template<std::size_t D, glm::qualifier Q>
		constexpr glm::vec<D, std::size_t, Q> get_dimension_multipliers(glm::vec<D, std::size_t, Q> extents)
		{
			auto sum = std::size_t{ 1 };
			auto multipliers = glm::vec<D, std::size_t, Q>(0);

			for (auto i : range(0, D))
			{
				multipliers[i] = sum;
				sum *= extents[i];
			}

			die_if(sum == 0);
			die_if(sum != get_data_size(extents));

			return multipliers;
		}

		template<std::size_t D, glm::qualifier Q>
		constexpr std::size_t to_index(glm::vec<D, std::size_t, Q> extents, glm::vec<D, std::size_t, Q> coords)
		{
			auto const multipliers = get_dimension_multipliers(extents);

			auto r = std::size_t{ 0 };
			for (auto i : range(0, D))
				r += coords[i] * multipliers[i];
			
			return r;
		}
		
		template<std::size_t D, glm::qualifier Q>
		constexpr glm::vec<D, std::size_t, Q> to_coords(glm::vec<D, std::size_t, Q> extents, std::size_t index)
		{
			auto const multipliers = get_dimension_multipliers(extents);

			auto r = glm::vec<D, std::size_t, Q>(0);

			for (auto i : range(0, D))
			{
				auto const d = (D - 1) - i;
				r.at(d) = index / multipliers.at(d);
				index %= multipliers.at(d);
			}
			
			return r;
		}

	} // grid_detail

	template<class T, std::size_t D, glm::qualifier Q = glm::defaultp>
	class grid
	{
	public:

		static_assert(D > 0, "grid<T, D>: dimensions must not be zero.");

		using value_type = T;
		using size_type = std::size_t;
		using extents_type = glm::vec<D, std::size_t, Q>;
		using data_type = std::vector<value_type>;
		using iterator = typename data_type::iterator;
		using const_iterator = typename data_type::const_iterator;
		using reverse_iterator = typename data_type::reverse_iterator;
		using const_reverse_iterator = typename data_type::const_reverse_iterator;

		constexpr grid():
			m_extents(0), m_data() { }

		explicit constexpr grid(extents_type extents):
			grid(extents, value_type()) { }

		explicit constexpr grid(extents_type extents, value_type const& value):
			m_extents(extents), m_data(size(), value) { }

		constexpr grid(grid const&) = default;
		constexpr grid& operator=(grid const&) = default;
		constexpr grid(grid&&) = default;
		constexpr grid& operator=(grid&&) = default;

		constexpr size_type size() const { return grid_detail::get_data_size(m_extents); }
		constexpr extents_type extents() const { return m_extents; }
		static constexpr std::size_t dimensions() { return D; }

		constexpr void resize(extents_type extents)
		{
			resize(extents, value_type());
		}

		constexpr void resize(extents_type extents, value_type const& value)
		{
			m_extents = extents;
			m_data.resize(size(), value);
		}

		constexpr void clear()
		{
			m_extents = extents_type(0);
			m_data.clear();
		}

		constexpr value_type& at(size_type index) { return m_data[index]; }
		constexpr value_type const& at(size_type index) const { return m_data[index]; }

		constexpr value_type& at(extents_type coords) { return at(to_index(coords)); }
		constexpr value_type const& at(extents_type coords) const { return at(to_index(coords)); }

		constexpr size_type to_index(extents_type coords) const { return grid_detail::to_index(m_extents, coords); }
		constexpr size_type to_coords(size_type index) const { return grid_detail::to_coords(m_extents, index); }

		iterator begin() { return m_data.begin(); }
		const_iterator begin() const { return m_data.begin(); }
		const_iterator cbegin() const { return m_data.cbegin(); }
		iterator end() { return m_data.end(); }
		const_iterator end() const { return m_data.end(); }
		const_iterator cend() const { return m_data.cend(); }
		reverse_iterator rbegin() { return m_data.rbegin(); }
		const_reverse_iterator rbegin() const { return m_data.rbegin(); }
		const_reverse_iterator crbegin() const { return m_data.crbegin(); }
		reverse_iterator rend() { return m_data.end(); }
		const_reverse_iterator rend() const { return m_data.end(); }
		const_reverse_iterator crend() const { return m_data.crend(); }

	private:

		extents_type m_extents;
		data_type m_data;
	};

	template<class T> using grid2 = grid<T, 2>;
	template<class T> using grid3 = grid<T, 3>;

} // bump
