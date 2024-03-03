#pragma once

#include <bump_math.hpp>

namespace rog
{
	
	enum class direction
	{
		UP_LEFT,
		UP,
		UP_RIGHT,
		LEFT,
		NONE,
		RIGHT,
		DOWN_LEFT,
		DOWN,
		DOWN_RIGHT,
	};

	glm::ivec2 get_direction_vector(direction dir);
	
	enum class stairs_direction
	{
		UP,
		DOWN,
	};

} // rog
