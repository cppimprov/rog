#include "ta_direction.hpp"

#include <bump_die.hpp>

namespace ta
{

	std::optional<direction> get_input_dir(bool input_up, bool input_down, bool input_left, bool input_right)
	{
		if (input_up && input_left)    return direction::up_left;
		if (input_up && input_right)   return direction::up_right;
		if (input_down && input_left)  return direction::down_left;
		if (input_down && input_right) return direction::down_right;

		if (input_up)    return direction::up;
		if (input_down)  return direction::down;
		if (input_left)  return direction::left;
		if (input_right) return direction::right;

		return { };
	}

	glm::vec2 dir_to_vec(direction dir)
	{
		switch (dir)
		{
		case direction::up:         return glm::vec2( 0.f,  1.f);
		case direction::down:       return glm::vec2( 0.f, -1.f);
		case direction::left:       return glm::vec2(-1.f,  0.f);
		case direction::right:      return glm::vec2( 1.f,  0.f);
		case direction::up_left:    return glm::vec2(-1.f,  1.f);
		case direction::up_right:   return glm::vec2( 1.f,  1.f);
		case direction::down_left:  return glm::vec2(-1.f, -1.f);
		case direction::down_right: return glm::vec2( 1.f, -1.f);
		}

		bump::die();
	}

	float dir_to_angle(direction dir)
	{
		switch (dir)
		{
		case direction::up:         return -90.f;
		case direction::down:       return 90.f;
		case direction::left:       return 0.f;
		case direction::right:      return 180.f;
		case direction::up_left:    return 0.f;
		case direction::up_right:   return -90.f;
		case direction::down_left:  return 90.f;
		case direction::down_right: return 180.f;
		}

		bump::die();
	}

	bool is_diagonal(direction dir)
	{
		switch (dir)
		{
		case direction::up:
		case direction::down:
		case direction::left:
		case direction::right:
			return false;
		case direction::up_left:
		case direction::up_right:
		case direction::down_left:
		case direction::down_right:
			return true;
		}

		bump::die();
	}

} // ta
