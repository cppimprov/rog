#include "ta_collision.hpp"

#include <bump_math.hpp>

#include <cassert>


namespace ta
{

	float reflect_pos(float position, float movement, float bound)
	{
		auto const diff = position - bound;
		auto const prop = diff / movement;
		auto const ref_prop = 1.f - prop;
		auto const ref_diff = ref_prop * movement;
		auto const pos = bound + ref_diff;
		return pos;
	}

	collision_result check_collision(world const& world, collision_object object, bool reflect, float delta_time_s)
	{
		// calculate movement
		auto const movement = dir_to_vec(object.m_direction) * object.m_speed * delta_time_s;

		// calculate new pos (without collision)
		auto const new_pos = object.m_position + movement;
		
		// adjust bounds for radius
		auto const bounds = ta::bounds
		{
			world.m_bounds.m_min + object.m_radius,
			world.m_bounds.m_max - object.m_radius,
		};

		assert(bounds.is_valid());

		bool collision = false;

		// iterate through vector components, reflecting if necessary
		for (auto i = std::size_t{ 0 }; i != object.m_position.length(); ++i)
		{
			if (new_pos[i] < bounds.m_min[i])
			{
				collision = true;

				if (reflect)
				{
					object.m_direction = reflect_dir(object.m_direction);
					object.m_position[i] = reflect_pos(object.m_position[i], movement[i], bounds.m_min[i]);
				}
				else
				{
					object.m_direction = direction::none;
					object.m_position[i] = bounds.m_min[i];
				}

				continue;
			}

			if (new_pos[i] > bounds.m_max[i])
			{
				collision = true;

				if (reflect)
				{
					object.m_direction = reflect_dir(object.m_direction);
					object.m_position[i] = reflect_pos(object.m_position[i], movement[i], bounds.m_max[i]);
				}
				else
				{
					object.m_direction = direction::none;
					object.m_position[i] = bounds.m_max[i];
				}

				continue;
			}

			object.m_position[i] = new_pos[i];
		}

		return collision_result{ collision, object.m_position, object.m_direction };
	}

} // ta
