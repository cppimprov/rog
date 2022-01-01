#pragma once

// from: https://github.com/klmr/cpp11-range

// Copyright 2012–2020 Konrad Rudolph

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cmath>
#include <iterator>
#include <type_traits>

namespace bump
{

	namespace detail
	{

		template <typename T>
		struct range_iter_base
		{

			using iterator_category = std::input_iterator_tag;
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = T *;
			using reference = T &;

			range_iter_base(T current) : current(current) {}

			T operator*() const { return current; }
			T const *operator->() const { return &current; }

			range_iter_base &operator++() { ++current; return *this; }
			range_iter_base operator++(int) { auto copy = *this; ++*this; return copy; }

			bool operator==(range_iter_base const &other) const { return current == other.current; }
			bool operator!=(range_iter_base const &other) const { return !(*this == other); }

		protected:

			T current;
		};

	} // detail

	template <typename T>
	struct step_range_proxy
	{
		struct iterator : detail::range_iter_base<T>
		{
			using detail::range_iter_base<T>::current;

			iterator(T current, T step): detail::range_iter_base<T>(current), step_(step) {}

			iterator &operator++() { current += step_; return *this; }
			iterator operator++(int) { auto copy = *this; ++*this; return copy; }

			// Loses commutativity. Iterator-based ranges are simply broken. :-(
			bool operator==(iterator const &other) const { return step_ > 0 ? current >= other.current : current < other.current; }
			bool operator!=(iterator const &other) const { return not(*this == other); }

			T step_;
		};

		step_range_proxy(T begin, T end, T step): begin_(begin, step), end_(end, step) {}

		iterator begin() const { return begin_; }
		iterator end() const { return end_; }

		std::size_t size() const
		{
			if (*end_ >= *begin_)
			{
				// Increasing and empty range
				if (begin_.step_ < T{0})
					return 0;
			}
			else
			{
				// Decreasing range
				if (begin_.step_ > T{0})
					return 0;
			}
			return std::ceil(std::abs(static_cast<double>(*end_ - *begin_) / begin_.step_));
		}

	private:

		iterator begin_;
		iterator end_;
	};

	template <typename T>
	struct range_proxy
	{
		struct iterator : detail::range_iter_base<T>
		{
			iterator(T current): detail::range_iter_base<T>(current) {}
		};

		range_proxy(T begin, T end) : begin_(begin), end_(end) {}

		step_range_proxy<T> step(T step) { return {*begin_, *end_, step}; }

		iterator begin() const { return begin_; }
		iterator end() const { return end_; }
		std::size_t size() const { return *end_ - *begin_; }

	private:

		iterator begin_;
		iterator end_;
	};

	template <typename T>
	struct step_inf_range_proxy
	{
		struct iterator : detail::range_iter_base<T>
		{
			using detail::range_iter_base<T>::current;

			iterator(T current = T(), T step = T()): detail::range_iter_base<T>(current), step(step) {}

			iterator &operator++() { current += step; return *this; }
			iterator operator++(int) { auto copy = *this; ++*this; return copy; }

			bool operator==(iterator const &) const { return false; }
			bool operator!=(iterator const &) const { return true; }

		private:

			T step;
		};

		step_inf_range_proxy(T begin, T step) : begin_(begin, step) {}

		iterator begin() const { return begin_; }
		iterator end() const { return iterator(); }

	private:

		iterator begin_;
	};

	template <typename T>
	struct infinite_range_proxy
	{
		struct iterator : detail::range_iter_base<T>
		{
			iterator(T current = T()) : detail::range_iter_base<T>(current) {}

			bool operator==(iterator const &) const { return false; }
			bool operator!=(iterator const &) const { return true; }
		};

		infinite_range_proxy(T begin) : begin_(begin) {}

		step_inf_range_proxy<T> step(T step) { return {*begin_, step}; }

		iterator begin() const { return begin_; }
		iterator end() const { return iterator(); }

	private:

		iterator begin_;
	};

	template <typename T, typename U>
	range_proxy<typename std::common_type<T, U>::type> range(T begin, U end)
	{
		using C = typename std::common_type<T, U>::type;
		return {static_cast<C>(begin), static_cast<C>(end)};
	}

	template <typename T>
	infinite_range_proxy<T> range(T begin)
	{
		return {begin};
	}

	namespace traits
	{

		template <typename C>
		struct has_size
		{
			template <typename T>
			static auto check(T *) ->
				typename std::is_integral<decltype(std::declval<T const>().size())>::type;

			template <typename>
			static auto check(...) -> std::false_type;

			using type = decltype(check<C>(0));
			static constexpr bool value = type::value;
		};

	} // namespace traits

	template <typename C, typename = typename std::enable_if<traits::has_size<C>::value>>
	auto indices(C const &cont) -> range_proxy<decltype(cont.size())>
	{
		return {0, cont.size()};
	}

	template <typename T, std::size_t N>
	range_proxy<std::size_t> indices(T (&)[N])
	{
		return {0, N};
	}

	template <typename T>
	range_proxy<typename std::initializer_list<T>::size_type>
	indices(std::initializer_list<T> &&cont)
	{
		return {0, cont.size()};
	}

} // bump
