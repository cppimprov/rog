#pragma once

#include <chrono>

namespace bump
{
	
	using clock_t = std::chrono::steady_clock;
	using duration_t = clock_t::duration;
	using time_point_t = clock_t::time_point;

	inline constexpr duration_t high_res_duration_from_seconds(float seconds)
	{
		return std::chrono::duration_cast<duration_t>(std::chrono::duration<float>(seconds));
	}

	inline constexpr float high_res_duration_to_seconds(duration_t duration)
	{
		return std::chrono::duration_cast<std::chrono::duration<float>>(duration).count();
	}
	
} // bump
