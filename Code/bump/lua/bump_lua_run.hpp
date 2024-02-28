#pragma once

#include "bump_lua_io.hpp"
#include "bump_lua_state.hpp"

namespace bump
{

	namespace lua
	{
	
		namespace detail
		{

			template<class... Rets, int... Is>
			std::tuple<Rets...> tuple_from_lua(state_view& lua, std::integer_sequence<int, Is...>)
			{
				static_assert(sizeof...(Rets) == sizeof...(Is));
				auto constexpr start = -static_cast<int>(sizeof...(Rets));
				return std::tuple<Rets...>{ from_lua<Rets>(lua, start + Is)... };
			}

			template<class... Rets>
			std::tuple<Rets...> tuple_from_lua(state_view& lua)
			{
				return tuple_from_lua<Rets...>(lua, std::make_integer_sequence<int, sizeof...(Rets)>{ });
			}

			template<class... Rets, class... Args>
			auto run_loaded(state_view& lua, Args&&... args)
			{
				(to_lua(lua, args), ...);

				auto constexpr num_args = sizeof...(Args);
				auto constexpr num_rets = sizeof...(Rets);

				if (lua.call(num_args, num_rets) != lua_status::ok)
					throw std::runtime_error(lua.pop_string());

				if constexpr (num_rets == 0)
				{
					return;
				}
				else if constexpr (num_rets == 1)
				{
					auto result = from_lua<Rets...>(lua, -1);
					lua.pop();
					return result;
				}
				else
				{
					auto results = tuple_from_lua<Rets...>(lua);
					lua.pop(num_rets);
					return results;
				}
			}

		} // detail

		template<class... Rets, class... Args>
		auto run(state_view& lua, std::string const& code, Args&&... args)
		{
			if (lua.load_string(code) != lua_status::ok)
				throw std::runtime_error(lua.pop_string());
			
			return detail::run_loaded<Rets...>(lua, std::forward<Args>(args)...);
		}

		template<class... Rets, class... Args>
		auto frun(state_view& lua, std::string const& file_path, Args&&... args)
		{
			if (lua.load_file(file_path) != lua_status::ok)
				throw std::runtime_error(lua.pop_string());
			
			return detail::run_loaded<Rets...>(lua, std::forward<Args>(args)...);
		}

	} // lua

} // bump
