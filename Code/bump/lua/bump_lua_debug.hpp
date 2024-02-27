#pragma once

#include "bump_lua_state.hpp"

#include <string>

namespace bump
{

	namespace lua
	{

		class state_view;

		std::string print_value(state_view const& state, int index);
		std::string print_stack(state_view const& state);
		std::string print_globals(state_view& state);
		
		std::string traceback(state_view& state, std::string const& prefix = "", int level = 0);

	} // lua

} // bump
