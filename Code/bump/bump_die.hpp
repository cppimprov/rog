#pragma once

namespace bump
{

	inline constexpr void die_if(bool condition) { if (condition) __debugbreak(); }
	[[noreturn]] inline constexpr void die() { die_if(true); }

} // bump
