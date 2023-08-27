#pragma once

#include "ta.hpp"

#include <bump_math.hpp>

namespace ta
{

	struct collision_object
	{
		glm::vec2 m_position;
		direction m_direction;
		float m_radius;
		float m_speed;
	};

	struct collision_result
	{
		bool m_collided;
		glm::vec2 m_position;
		direction m_direction;
	};

	collision_result check_collision(world const& world, collision_object object, bool reflect, float delta_time_s);

} // ta
