#pragma once

#include "bump_lua_io.hpp"

#include <array>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

namespace bump
{

	namespace lua
	{

		// STRINGS

		template<>
		struct to_lua_impl<std::string>
		{
			static void to_lua(state_view lua, std::string const& value) { lua.push_string(value); }
		};

		template<>
		struct to_lua_impl<std::string_view>
		{
			static void to_lua(state_view lua, std::string_view value) { lua.push_string(value); }
		};

		template<>
		struct to_lua_impl<char const*>
		{
			static void to_lua(state_view lua, char const* value) { lua.push_string(value); }
		};

		template<>
		struct from_lua_impl<std::string>
		{
			static std::string from_lua(state_view lua, int index) { return std::string(lua.to_string(index)); }
		};

		// ARRAY

		template<class T, std::size_t N>
		struct to_lua_impl<std::array<T, N>>
		{
			static void to_lua(state_view lua, std::array<T, N> const& arr)
			{
				lua.push_new_table(detail::narrow_cast<int>(N));

				for (auto i = std::size_t{ 0 }; i != N; ++i)
				{
					luups::to_lua(lua, arr[i]);
					lua.set_field(-2, detail::narrow_cast<lua_integer>(i + 1));
				}
			}
		};

		template<class T, std::size_t N>
		struct from_lua_impl<std::array<T, N>>
		{

			// note: iterates indices covering the entire array so holes (nil values)
			// are ignored, and left as default constructed values.
			// if values are present, they are expected to be the correct type (for now).

			static std::array<T, N> from_lua(state_view lua, int index)
			{
				auto arr = std::array<T, N>{ };

				for (auto i = std::size_t{ 0 }; i != N; ++i)
				{
					lua.push_field_raw(index, detail::narrow_cast<lua_integer>(i + 1));

					if (lua.is_nil(-1))
					{
						lua.pop();
						continue;
					}

					arr[i] = luups::from_lua<T>(lua);
					lua.pop();
				}

				return arr;
			}
		};

		// VECTOR

		template<class T>
		struct to_lua_impl<std::vector<T>>
		{
			static void to_lua(state_view lua, std::vector<T> const& vec)
			{
				lua.push_new_table(detail::narrow_cast<int>(vec.size()));

				for (auto i = std::size_t{ 0 }; i != vec.size(); ++i)
				{
					luups::to_lua(lua, vec[i]);
					lua.set_field(-2, detail::narrow_cast<lua_integer>(i + 1));
				}
			}
		};

		template<class T>
		struct from_lua_impl<std::vector<T>>
		{
			// note: iterates from index 1 until the first nil value
			// values are expected to be the correct type.

			static std::vector<T> from_lua(state_view lua, int index)
			{
				auto vec = std::vector<T>{ };

				for (auto i = lua_integer{ 1 }; ; ++i)
				{
					lua.push_field_raw(index, i);

					if (lua.is_nil(-1))
					{
						lua.pop();
						break;
					}

					vec.push_back(luups::from_lua<T>(lua));
					lua.pop();
				}

				return vec;
			}
		};

		// MAP

		template<class K, class V>
		struct to_lua_impl<std::map<K, V>>
		{
			static void to_lua(state_view lua, std::map<K, V> const& map)
			{
				lua.push_new_table();

				for (auto const& [key, value] : map)
				{
					luups::to_lua(lua, key);
					luups::to_lua(lua, value);
					lua.set_field(-3);
				}
			}
		};

		template<class K, class V>
		struct from_lua_impl<std::map<K, V>>
		{
			static std::map<K, V> from_lua(state_view lua, int )
			{
				auto map = std::map<K, V>{ };

				lua.push_nil();

				while (lua.next_field(-2))
				{
					auto const k = luups::from_lua<K>(lua, -2);
					auto const v = luups::from_lua<V>(lua, -1);
					map[k] = v;

					lua.pop();
				}

				return map;
			}
		};

		// TUPLE

		template<class... Ts>
		struct to_lua_impl<std::tuple<Ts...>>
		{
			template<class T, int I>
			static void to_lua_value(state_view& lua, T const& value)
			{
				luups::to_lua(lua, value);
				lua.set_field(-2, I);
			}

			template<class... Ts, int... Is>
			static void to_lua_values(state_view& lua, std::tuple<Ts...> const& tup, std::integer_sequence<int, Is...>)
			{
				(to_lua_value<Ts, Is + 1>(lua, std::get<Is>(tup)), ...);
			}

			static void to_lua(state_view lua, std::tuple<Ts...> const& tup)
			{
				lua.push_new_table();

				to_lua_values(lua, tup, std::make_integer_sequence<int, sizeof...(Ts)>{ });
			}
		};

		template<class... Ts>
		struct from_lua_impl<std::tuple<Ts...>>
		{
			// note: iterates indices covering the entire tuple so holes (nil values)
			// are ignored, and filled with default constructed values.
			// if values are present, they are expected to be the correct type (for now).

			template<class T, int I>
			static T from_lua_value(state_view& lua, int index)
			{
				lua.push_field_raw(index, I);

				if (lua.is_nil(-1))
				{
					lua.pop();
					return T{ };
				}

				auto const value = luups::from_lua<T>(lua, -1);
				lua.pop();
				return value;
			}

			template<class... Ts, int... Is>
			static std::tuple<Ts...> from_lua_values(state_view& lua, int index, std::integer_sequence<int, Is...>)
			{
				return { from_lua_value<Ts, Is + 1>(lua, index)... };
			}

			static std::tuple<Ts...> from_lua(state_view lua, int index)
			{
				return from_lua_values<Ts...>(lua, index, std::make_integer_sequence<int, sizeof...(Ts)>{ });
			}
		};
	
	} // lua

} // bump
