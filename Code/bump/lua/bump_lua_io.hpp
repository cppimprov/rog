#pragma once

#include "bump_lua_state.hpp"

namespace bump
{

	namespace lua
	{

		template<class T> struct to_lua_impl;

		template<class T>
		void to_lua(state_view lua, T&& value)
		{
			return to_lua_impl<std::decay_t<T>>::to_lua(lua, std::forward<T>(value));
		}

		template<class T> struct from_lua_impl;

		template<class T>
		T from_lua(state_view lua, int index)
		{
			return from_lua_impl<std::decay_t<T>>::from_lua(lua, index);
		}

	} // lua

} // bump
