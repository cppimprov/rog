#include "rog_ecs.hpp"

#include "rog_feature.hpp"
#include "rog_level.hpp"

#include <bump_log.hpp>

namespace rog
{

	entt::entity player_create_entity(entt::registry& registry)
	{
		auto player = registry.create();
		
		registry.emplace<c_player_tag>(player);
		registry.emplace<c_position>(player, glm::ivec2(0));
		registry.emplace<c_visual>(player, screen_cell{ '@', colors::yellow, colors::black });
		registry.emplace<c_actor>(player, 100); // start with enough energy to move

		return player;
	}

	bool player_can_use_stairs(level& level, stairs_direction dir)
	{
		auto const& grid = level.m_grid;
		auto const level_size = grid.extents();

		auto const& pos = level.m_registry.get<c_position>(level.m_player);

		bump::die_if(level_size.x == 0 || level_size.y == 0);
		bump::die_if(pos.m_pos.x >= level_size.x || pos.m_pos.y >= level_size.y);

		if (dir == stairs_direction::UP)
		{
			if (!(grid.at(pos.m_pos).m_flags & feature::flags::STAIRS_UP))
			{
				bump::log_info("There are no upward stairs here.");
				return false;
			}

			return true;
		}
		else
		{
			if (!(grid.at(pos.m_pos).m_flags & feature::flags::STAIRS_DOWN))
			{
				bump::log_info("There are no downward stairs here.");
				return false;
			}

			return true;
		}
	}

	entt::entity monster_create_entity(entt::registry& registry)
	{
		auto monster = registry.create();

		registry.emplace<c_monster_tag>(monster);
		registry.emplace<c_position>(monster, glm::ivec2(0));
		registry.emplace<c_visual>(monster, screen_cell{ 'T', colors::green, colors::black });
		registry.emplace<c_actor>(monster, 0); // start without energy to move (player has advantage here)
		
		return monster;
	}
	
	void monster_move(level& level, entt::entity monster, c_position& pos, random::rng_t& rng)
	{
		// todo: random direction function? (and ignore 5?)
		auto const dir_idx = random::rand_range(rng, 0, 8);
		auto const dir = static_cast<direction>(dir_idx);

		(void)move_actor(level, monster, pos, dir); // todo: try a different direction?
	}

} // rog
