#pragma once

#include <cstdlib>

#if defined(_MSC_VER)

namespace bump
{

	inline constexpr void die_if(bool condition) { if (condition) __debugbreak(); }
	[[noreturn]] inline constexpr void die() { die_if(true); }

} // bump

#else

#include <signal.h>

namespace bump
{
	
	inline constexpr void die_if(bool condition) { if (condition) std::abort(); }
	[[noreturn]] inline constexpr void die() { std::abort(); }
	
} // bump


#endif
