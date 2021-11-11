#pragma once

#include <glm/glm.hpp>

namespace rog
{
	
	enum class direction
	{
		LEFT_UP,
		UP,
		RIGHT_UP,
		LEFT,
		NONE,
		RIGHT,
		LEFT_DOWN,
		DOWN,
		RIGHT_DOWN,
	};

	glm::ivec2 get_direction_vector(direction dir);
	
} // rog
