#pragma once

#include <bump_math.hpp>

#include <cstdint>
#include <optional>

namespace ta
{

	enum class direction
	{
		up,
		down,
		left,
		right,
		up_left,
		up_right,
		down_left,
		down_right,
	};

	std::optional<direction> get_input_dir(bool input_up, bool input_down, bool input_left, bool input_right);
	glm::vec2 dir_to_vec(direction dir);
	float dir_to_angle(direction dir);
	bool is_diagonal(direction dir);

} // ta
