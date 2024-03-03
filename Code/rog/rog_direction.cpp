#include "rog_direction.hpp"

#include "bump_die.hpp"

namespace rog
{
	
	glm::ivec2 get_direction_vector(direction dir)
	{
		switch (dir)
		{
		case direction::UP_LEFT:    return { -1, -1 };
		case direction::UP:         return {  0, -1 };
		case direction::UP_RIGHT:   return {  1, -1 };
		case direction::LEFT:       return { -1,  0 };
		case direction::NONE:       return {  0,  0 };
		case direction::RIGHT:      return {  1,  0 };
		case direction::DOWN_LEFT:  return { -1,  1 };
		case direction::DOWN:       return {  0,  1 };
		case direction::DOWN_RIGHT: return {  1,  1 };
		}

		bump::die();
	}
	
} // rog
