#pragma once

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>

namespace bump
{

	/**
	 * Stack-based string storage
	 * 
	 * A string class with a fixed maximum size using a std::array for underlying storage. No
	 * modification is supported (apart from clearing the stored string).
	 * 
	 * Only comparison with fixed_basic_strings of the same type is supported. Conversion to
	 * std::stringview using the .sv() function should be used as required.
	 */
	
	template<class CharT, std::size_t N>
	class basic_static_string
	{
		using data_type = std::array<std::remove_cv_t<CharT>, N + 1>;
		using init_list_type = std::initializer_list<CharT>;
		using string_type = std::basic_string<CharT>;
		using string_view_type = std::basic_string_view<CharT>;
		using c_str_type = CharT const*;
		
	public:

		using value_type = std::remove_cv_t<CharT>;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type&;
		using const_reference = value_type const&;
		using pointer = value_type*;
		using const_pointer = value_type const*;
		using iterator = typename data_type::iterator;
		using const_iterator = typename data_type::const_iterator;
		using reverse_iterator = typename data_type::reverse_iterator;
		using const_reverse_iterator = typename data_type::const_reverse_iterator;

		// CONSTRUCTORS
		constexpr basic_static_string():
			m_size(0)
		{
			ensure_null();
		}

		constexpr explicit basic_static_string(size_type count, value_type ch):
			basic_static_string()
		{
			if (count > max_size())
				throw std::length_error("`count` exceeds max_size()");
			
			std::fill_n(m_data.begin(), count, ch);

			m_size = count;
			ensure_null();
		}

		constexpr basic_static_string(c_str_type s):
			basic_static_string()
		{
			auto dst = m_data.begin();
			auto const end = m_data.begin() + max_size();

			while (dst != end && *s != '\0')
				*dst++ = *s++;
			
			if (*s != '\0')
				throw std::length_error("length of string exceeds max_size()");

			m_size = dst - m_data.begin();
			ensure_null();
		}

		constexpr explicit basic_static_string(init_list_type s):
			basic_static_string()
		{
			if (s.size() > max_size())
				throw std::length_error("initializer_list::size() exceeds max_size()");

			std::copy(s.begin(), s.end(), m_data.begin());

			m_size = s.size();
			ensure_null();
		}

		constexpr explicit basic_static_string(string_type const& s):
			basic_static_string()
		{
			if (s.size() > max_size())
				throw std::length_error("std::basic_string::size() exceeds max_size()");

			std::copy(s.begin(), s.end(), m_data.begin());
			
			m_size = s.size();
			ensure_null();
		}

		template<class SVT, 
			typename = std::enable_if_t<
				std::is_convertible_v<const SVT&, string_view_type> && 
				!std::is_convertible_v<const SVT&, c_str_type>> >
		constexpr explicit basic_static_string(SVT const& s):
			basic_static_string()
		{
			if (s.size() > max_size())
				throw std::length_error("std::basic_string_view::size() exceeds max_size()");

			std::copy(s.begin(), s.end(), m_data.begin());

			m_size = s.size();
			ensure_null();
		}
		
		template<class InputIt>
		constexpr explicit basic_static_string(InputIt first, InputIt last):
			basic_static_string()
		{
			auto dst = m_data.begin();
			auto const end = m_data.begin() + max_size();

			while (dst != end && first != last)
				*dst++ = *first++;
			
			if (first != last)
				throw std::length_error("std::distance(first, last) exceeds max_size()");

			m_size = dst - m_data.begin();
			ensure_null();
		}

		template<class C, std::size_t S>
		constexpr basic_static_string(basic_static_string<C, S> const& other):
			basic_static_string()
		{
			if (other.size() > max_size())
				throw std::length_error("basic_static_string<C, S> content exceeds max_size()");

			std::copy_n(other.begin(), other.size(), begin());

			m_size = other.size();
			ensure_null();
		}
		
		// COPY AND MOVE
		constexpr basic_static_string(basic_static_string const& other) = default;
		constexpr basic_static_string& operator=(basic_static_string const& other) = default;

		constexpr basic_static_string(basic_static_string&& other) = default;
		constexpr basic_static_string& operator=(basic_static_string&& other) = default;

		// ELEMENT ACCESS
		constexpr reference at(size_type pos) { return m_data.at(pos); }
		constexpr const_reference at(size_type pos) const { return m_data.at(pos); }
		constexpr reference operator[](size_type pos) { return m_data[pos]; }
		constexpr const_reference operator[](size_type pos) const { return m_data[pos]; }

		constexpr reference front() { return m_data.front(); }
		constexpr const_reference front() const { return m_data.front(); }
		constexpr reference back() { return at(size() - 1); }
		constexpr const_reference back() const { return at(size() - 1); }

		// POINTER ACCESS AND CONVERSION
		constexpr pointer data() noexcept { return m_data.data(); }
		constexpr const_pointer data() const noexcept { return m_data.data(); }
		constexpr const_pointer c_str() const noexcept { return data(); }

		constexpr string_type str() const noexcept { return string_type(begin(), end()); }
		constexpr string_view_type sv() const noexcept { return string_view_type(data(), size()); }

		// ITERATORS
		constexpr iterator begin() { return m_data.begin(); }
		constexpr const_iterator begin() const { return m_data.begin(); }
		constexpr const_iterator cbegin() const { return m_data.cbegin(); }
		
		constexpr iterator end() { return m_data.begin() + size(); }
		constexpr const_iterator end() const { return m_data.begin() + size(); }
		constexpr const_iterator cend() const { return m_data.cbegin() + size(); }

		constexpr reverse_iterator rbegin() { return make_reverse_iterator(end()); }
		constexpr const_reverse_iterator rbegin() const { return make_reverse_iterator(end()); }
		constexpr const_reverse_iterator crbegin() const { return make_reverse_iterator(cend()); }
		
		constexpr reverse_iterator rend() { return m_data.rend(); }
		constexpr const_reverse_iterator rend() const { return m_data.rend(); }
		constexpr const_reverse_iterator crend() const { return m_data.rend(); }

		// SIZE
		[[nodiscard]] constexpr bool empty() const noexcept { return (size() == 0); }
		constexpr size_type size() const noexcept { return m_size; }
		static constexpr size_type max_size() noexcept { return N; }
		static constexpr size_type capacity() noexcept { return max_size(); }

		// OPERATIONS
		constexpr void clear() noexcept
		{
			m_size = 0;
			ensure_null();
		}

		constexpr void push_back(value_type ch)
		{
			if (size == max_size())
				throw std::length_error("push_back() would exceed max_size()");
			
			m_data[m_size++] = ch;
			ensure_null();
		}

		constexpr void pop_back(value_type ch)
		{
			if (empty())
				throw std::runtime_error("pop_back() called on empty container");

			--m_size;
			ensure_null();
		}

		constexpr void resize(size_type rsize)
		{
			resize(rsize, value_type());
		}

		constexpr void resize(size_type rsize, value_type ch)
		{
			if (rsize > max_size())
				throw std::length_error("requested size exceeds max_size()");
			
			if (rsize < size())
			{
				m_size = rsize;
				ensure_null();
			}
			else
			{
				while (size() != rsize)
					m_data[m_size++] = ch;
				
				ensure_null();
			}
		}

	private:

		void ensure_null() { m_data[m_size] = '\0'; }
		
		std::size_t m_size;
		data_type m_data;
	};

	// TYPEDEFS
	template<std::size_t N>
	using static_string = basic_static_string<char, N>;

	template<std::size_t N>
	using static_wstring = basic_static_string<wchar_t, N>;

	// COMPARISON
	template<class CharT, std::size_t N, std::size_t M>
	bool operator==(basic_static_string<CharT, N> const& a, basic_static_string<CharT, M> const& b)
	{
		return std::equal(a.cbegin(), a.cend(), b.cbegin(), b.cend());
	}
	template<class CharT, std::size_t N, std::size_t M>
	bool operator!=(basic_static_string<CharT, N> const& a, basic_static_string<CharT, M> const& b)
	{
		return !(a == b);
	}
	template<class CharT, std::size_t N, std::size_t M>
	bool operator<(basic_static_string<CharT, N> const& a, basic_static_string<CharT, M> const& b)
	{
		return std::lexicographical_compare(a.cbegin(), a.cend(), b.cbegin(), b.cend());
	}
	template<class CharT, std::size_t N, std::size_t M>
	bool operator>(basic_static_string<CharT, N> const& a, basic_static_string<CharT, M> const& b)
	{
		return (b < a);
	}
	template<class CharT, std::size_t N, std::size_t M>
	bool operator<=(basic_static_string<CharT, N> const& a, basic_static_string<CharT, M> const& b)
	{
		return !(b < a);
	}
	template<class CharT, std::size_t N, std::size_t M>
	bool operator>=(basic_static_string<CharT, N> const& a, basic_static_string<CharT, M> const& b)
	{
		return !(a < b);
	}

} // bump

namespace std
{
	
	template<class CharT, std::size_t N>
	struct hash<bump::basic_static_string<CharT, N>>
	{
		std::size_t operator()(bump::basic_static_string<CharT, N> const& s) const
		{
			return std::hash<std::basic_string_view<CharT>>()(s.sv());
		}
	};
	
} // std
