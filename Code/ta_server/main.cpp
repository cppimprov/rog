
#include <ta.hpp>

#include <bump_log.hpp>
#include <bump_net.hpp>
#include <bump_math.hpp>
#include <bump_timer.hpp>

#include <iostream>
#include <map>
#include <string>
#include <system_error>

#include <iostream>

struct collision_object
{
	glm::vec3 m_position;
	ta::direction m_direction;
	float m_radius;
};

struct collision_result
{
	bool m_collided;
	glm::vec3 m_position;
	ta::direction m_direction;
};

float reflect_pos(float position, float movement, float bound)
{
	auto const diff = position - bound;
	auto const prop = diff / movement;
	auto const ref_prop = 1.f - prop;
	auto const ref_diff = ref_prop * movement;
	auto const pos = bound + ref_diff;

	return pos;
}

collision_result check_collision(ta::world const& world, collision_object object, bool reflect, float delta_time_s)
{
	// calculate movement
	auto const movement = ta::dir_to_vec(object.m_direction) * delta_time_s;

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
	for (auto i = std::size_t{ 0 }; i != std::size_t{ 3 }; ++i)
	{
		if (new_pos[i] < bounds.m_min[i])
		{
			collision = true;

			if (reflect)
			{
				object.m_direction = ta::reflect_dir(object.m_direction);
				object.m_position[i] = reflect_pos(object.m_position[i], movement[i], bounds.m_min[i]);
			}
			else
			{
				object.m_direction = ta::direction::none;
				object.m_position[i] = bounds.m_min[i];
			}
		}

		if (new_pos[i] > bounds.m_max[i])
		{
			collision = true;

			if (reflect)
			{
				object.m_direction = ta::reflect_dir(object.m_direction);
				object.m_position[i] = reflect_pos(object.m_position[i], movement[i], bounds.m_max[i]);
			}
			else
			{
				object.m_direction = ta::direction::none;
				object.m_position[i] = bounds.m_max[i];
			}
		}
	}

	return collision_result{ collision, object.m_position, object.m_direction };
}

int main()
{
	using namespace bump;

	auto const max_hp = ta::hp_t{ 100 };
	auto const player_speed = 1.f;
	auto const player_radius = 0.5f;
	auto const bullet_radius = 0.05f;
	auto const powerup_radius = 0.25f;
	auto const powerup_duration = 10.f;

	auto world = ta::world{ glm::vec3{ 0.f, 0.f, 0.f }, glm::vec3{ 10.f, 10.f, 10.f } };
	world.m_players.push_back(ta::player{ 0, max_hp, glm::vec3{ 0.f, 0.f, 0.f }, ta::direction::up });
	world.m_players.push_back(ta::player{ 1, max_hp, glm::vec3{ 1.f, 0.f, 0.f }, ta::direction::up });
	world.m_players.push_back(ta::player{ 2, max_hp, glm::vec3{ 2.f, 0.f, 0.f }, ta::direction::up });
	world.m_players.push_back(ta::player{ 3, max_hp, glm::vec3{ 3.f, 0.f, 0.f }, ta::direction::up });

	auto timer = bump::frame_timer();

	while (true)
	{
		auto const delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(timer.get_last_frame_time()).count();

		// update player positions
		for (auto& player : world.m_players)
		{
			auto result = check_collision(world, { player.m_position, player.m_direction, player_radius }, false, delta_time);
			player.m_position = result.m_position;
			player.m_direction = result.m_direction;
		}

		// update bullet positions
		for (auto& bullet : world.m_bullets)
		{
			auto result = check_collision(world, { bullet.m_position, bullet.m_direction, bullet_radius }, false, delta_time);
			bullet.m_position = result.m_position;
			bullet.m_direction = result.m_direction;
		}

		// remove expired bullets
		std::erase_if(world.m_bullets,
			[] (auto const& b) { return b.m_lifetime <= 0.f; });

		// update powerup timers
		for (auto& player : world.m_players)
		{
			for (auto& powerup_timer : player.m_powerup_timers)
				powerup_timer.second -= delta_time;
				
			// remove expired powerups
			std::erase_if(player.m_powerup_timers,
				[] (auto const& p) { return p.second <= 0.f; });
		}

		// check for powerup pickups
		for (auto& player : world.m_players)
		{
			for (auto& powerup : world.m_powerups)
			{
				auto const dist = glm::distance(player.m_position, powerup.m_position);

				if (dist < player_radius + powerup_radius)
					player.m_powerup_timers[powerup.m_type] = powerup_duration;
			}
		}

		// check for player damage
		for (auto& player : world.m_players)
		{
			for (auto& bullet : world.m_bullets)
			{
				auto const dist = glm::distance(player.m_position, bullet.m_position);

				if (dist < player_radius + bullet_radius)
					player.m_hp -= bullet.m_damage;
			}
		}

		// check for player deaths (remove dead players)
		std::erase_if(world.m_players,
			[] (auto const& p) { return p.m_hp <= 0; });

		// ...

		timer.tick();
	}

	std::cout << "done!" << std::endl;
}
