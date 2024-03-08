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

		template<std::size_t D, class L, glm::qualifier Q>
		constexpr L get_data_size(glm::vec<D, L, Q> extents)
		{
			auto r = L{ 1 };

			for (auto i : range(0, D))
				r *= extents[i];
			
			return r;
		}

		template<std::size_t D, class L, glm::qualifier Q>
		constexpr glm::vec<D, L, Q> get_dimension_multipliers(glm::vec<D, L, Q> extents)
		{
			auto sum = L{ 1 };
			auto multipliers = glm::vec<D, L, Q>(0);

			for (auto i : range(0, D))
			{
				multipliers[i] = sum;
				sum *= extents[i];
			}

			die_if(sum == 0);
			die_if(sum != get_data_size(extents));

			return multipliers;
		}

		template<std::size_t D, class L, glm::qualifier Q>
		constexpr L to_index(glm::vec<D, L, Q> extents, glm::vec<D, L, Q> coords)
		{
			auto const multipliers = get_dimension_multipliers(extents);

			auto r = L{ 0 };

			for (auto i : range(0, D))
				r += coords[i] * multipliers[i];
			
			return r;
		}
		
		template<std::size_t D, class L, glm::qualifier Q>
		constexpr glm::vec<D, L, Q> to_coords(glm::vec<D, L, Q> extents, L index)
		{
			auto const multipliers = get_dimension_multipliers(extents);

			auto r = glm::vec<D, L, Q>(0);

			for (auto i : range(0, D))
			{
				auto const d = (D - 1) - i;
				r.at(d) = index / multipliers.at(d);
				index %= multipliers.at(d);
			}
			
			return r;
		}

	} // grid_detail

	template<class T, std::size_t D, class C = glm::vec<D, glm::length_t, glm::defaultp> >
	class grid
	{
	public:

		static_assert(D > 0, "grid<T, D>: dimensions must not be zero.");

		using value_type = T;
		using size_type = typename C::length_type;
		using coords_type = C;
		using data_type = std::vector<value_type>;
		using iterator = typename data_type::iterator;
		using const_iterator = typename data_type::const_iterator;
		using reverse_iterator = typename data_type::reverse_iterator;
		using const_reverse_iterator = typename data_type::const_reverse_iterator;

		constexpr grid():
			m_extents(0), m_data() { }

		explicit constexpr grid(coords_type extents):
			grid(extents, value_type()) { }

		explicit constexpr grid(coords_type extents, value_type const& value):
			m_extents(extents), m_data(size(), value) { }

		constexpr grid(grid const&) = default;
		constexpr grid& operator=(grid const&) = default;
		constexpr grid(grid&&) = default;
		constexpr grid& operator=(grid&&) = default;

		constexpr size_type size() const { return grid_detail::get_data_size(m_extents); }
		constexpr coords_type extents() const { return m_extents; }
		static constexpr std::size_t dimensions() { return D; }

		constexpr void resize(coords_type extents)
		{
			resize(extents, value_type());
		}

		constexpr void resize(coords_type extents, value_type const& value)
		{
			m_extents = extents;
			m_data.resize(size(), value);
		}

		constexpr void clear()
		{
			m_extents = coords_type(0);
			m_data.clear();
		}

		constexpr value_type& at(size_type index) { return m_data[index]; }
		constexpr value_type const& at(size_type index) const { return m_data[index]; }

		constexpr value_type& at(coords_type coords) { return at(to_index(coords)); }
		constexpr value_type const& at(coords_type coords) const { return at(to_index(coords)); }

		constexpr size_type to_index(coords_type coords) const { return grid_detail::to_index(m_extents, coords); }
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

		coords_type m_extents;
		data_type m_data;
	};

	template<class T> using grid2 = grid<T, 2>;
	template<class T> using grid3 = grid<T, 3>;

} // bump
