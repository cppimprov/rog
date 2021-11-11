#include "rog_direction.hpp"

#include "bump_die.hpp"

namespace rog
{
	
	glm::ivec2 get_direction_vector(direction dir)
	{
		switch (dir)
		{
		case direction::LEFT_UP:    return { -1, -1 };
		case direction::UP:         return {  0, -1 };
		case direction::RIGHT_UP:   return {  1, -1 };
		case direction::LEFT:       return { -1,  0 };
		case direction::NONE:       return {  0,  0 };
		case direction::RIGHT:      return {  1,  0 };
		case direction::LEFT_DOWN:  return { -1,  1 };
		case direction::DOWN:       return {  0,  1 };
		case direction::RIGHT_DOWN: return {  1,  1 };
		}

		bump::die();
	}
	
} // rog
