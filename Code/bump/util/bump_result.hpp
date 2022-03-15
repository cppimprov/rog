#pragma once

#include "bump_die.hpp"

#include <utility>
#include <variant>

namespace bump
{

	template<class T> struct ok { T m_value; };
	template<> struct ok<void> { };
	
	template<class E> struct err { E m_error; };
	
	template<class T, class E>
	class result
	{
	public:

		using value_type = T;
		using error_type = E;
		
		result(ok<T> value): m_value(std::in_place_index_t<0>(), std::move(value.m_value)) { }
		result(err<E> error): m_value(std::in_place_index_t<1>(), std::move(error.m_error)) { }

		result& operator=(ok<T> value) { m_value.emplace<0>(std::move(value.m_value)); return *this; }
		result& operator=(err<E> error) { m_value.emplace<1>(std::move(error.m_error)); return *this; }

		result(result const& other) = default;
		result& operator=(result const& other) = default;
		result(result&& other) = default;
		result& operator=(result&& other) = default;

		bool has_value() const { return m_value.index() == 0; }
		explicit operator bool() const { return has_value(); }

		value_type& value() { die_if(!has_value()); return std::get<0>(m_value); }
		value_type const& value() const { die_if(!has_value()); return std::get<0>(m_value); }

		error_type& error() { die_if(has_value()); return std::get<1>(m_value); }
		error_type const& error() const { die_if(has_value()); return std::get<1>(m_value); }

		friend bool operator==(result const& a, result const& b) { return a.m_value == b.m_value; }
		friend bool operator!=(result const& a, result const& b) { return !(a == b); }

	private:
		
		std::variant<T, E> m_value;
	};

	template<class E>
	class result<void, E>
	{
	public:

		using value_type = void;
		using error_type = E;
		
		result(ok<void>): m_value(std::in_place_index_t<0>(), std::monostate()) { }
		result(err<E> error): m_value(std::in_place_index_t<1>(), std::move(error.m_error)) { }

		result& operator=(ok<void> value) { m_value.emplace<0>(std::monostate()); return *this; }
		result& operator=(err<E> error) { m_value.emplace<1>(std::move(error.m_error)); return *this; }

		result(result const& other) = default;
		result& operator=(result const& other) = default;
		result(result&& other) = default;
		result& operator=(result&& other) = default;

		bool has_value() const { return (m_value.index() == 0); }
		explicit operator bool() const { return has_value(); }

		void value() { die_if(!has_value()); }

		error_type& error() { die_if(has_value()); return std::get<1>(m_value); }
		error_type const& error() const { die_if(has_value()); return std::get<1>(m_value); }

		friend bool operator==(result const& a, result const& b) { return a.m_value == b.m_value; }
		friend bool operator!=(result const& a, result const& b) { return !(a == b); }

	private:

		std::variant<std::monostate, E> m_value;
	};

	template<class T> ok<T> make_ok(T t) { return { std::move(t) }; }
	inline ok<void> make_ok() { return { }; }

	template<class E> err<E> make_err(E e) { return { std::move(e) }; }
	
} // bump
