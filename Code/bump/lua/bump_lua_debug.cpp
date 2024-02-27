#include "bump_lua_debug.hpp"

#include "bump_die.hpp"
#include "bump_lua_state.hpp"

#include <array>
#include <bit>
#include <charconv>
#include <cstdint>
#include <string>

namespace bump
{

	namespace lua
	{

		std::string print_value(state_view const& state, int index)
		{
			auto const type = state.get_type(index);

			auto const to_hex_string = [&] (auto val)
			{
				auto const leading_zero_bits = std::countl_zero(val);
				auto const leading_zero_hex = leading_zero_bits / 4;

				auto chars = std::array<char, 2 + sizeof(val) * 2 + 1>();
				auto result = std::to_chars(
					chars.data() + 2 + leading_zero_hex, 
					chars.data() + 2 + (sizeof(val) * 2), val, 16);
				die_if(result.ec != std::errc());
				chars[0] = '0';
				chars[1] = 'x';
				return std::string(chars.data(), chars.size());
			};

			auto const to_pointer = [&] (int index)
			{
				return to_hex_string(reinterpret_cast<std::uintptr_t>(lua_topointer(state.handle(), index)));
			};
			
			switch (type)
			{
			case type::none: return "none";
			case type::nil: return "nil";
			case type::boolean: return (state.to_boolean(index) ? "true" : "false");
			case type::lightuserdata: return to_pointer(index);
			case type::number: return std::to_string(state.to_number(index));
			case type::string: return std::string(state.to_string(index));
			case type::table: return to_pointer(index);
			case type::function: return to_pointer(index);
			case type::userdata: return to_pointer(index);
			case type::thread: return to_pointer(index);
			}

			die();
		}

		std::string print_stack(state_view const& state)
		{
			auto result = std::string();
			result.append("stack (size " + std::to_string(state.size()) + "):\n");

			for (int i = 1; i <= state.size(); ++i)
			{
				result.append("" + std::to_string(i) + ":\t ");
				result.append(lua::to_string(state.get_type(i)) + "\t" + print_value(state, i) + "\n");
			}

			return result;
		}

		std::string print_globals(state_view& state)
		{
			auto result = std::string();
			result.append("globals:\n");

			state.push_globals_table();
			state.push_nil();

			while (state.next_field(-2))
			{
				result.append(print_value(state, -2) + " -> " + print_value(state, -1) + "\n");
				state.pop();
			}

			state.pop();

			return result;
		}
		
		std::string traceback(state_view& state, std::string const& prefix, int level)
		{
			luaL_traceback(state.handle(), state.handle(), prefix.data(), level);
			return state.pop_string();
		}
	
	} // lua

} // bump
