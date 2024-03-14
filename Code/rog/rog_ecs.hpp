#pragma once

#include "rog_direction.hpp"
#include "rog_random.hpp"
#include "rog_screen.hpp"

#include <bump_math.hpp>

#include <entt.hpp>

namespace rog
{

	struct level;

	// COMPONENTS:

	struct c_player_char_info
	{
		std::string m_name;
		std::string m_title;
	};

	struct c_xp
	{
		std::uint32_t m_xp = 0;
		std::uint32_t m_level = 1;
	};

	struct c_stats
	{
		std::int32_t m_str = 10;
		std::int32_t m_dex = 10;
		std::int32_t m_con = 10;
		std::int32_t m_int = 10;
		std::int32_t m_wis = 10;
		std::int32_t m_cha = 10;
	};

	struct c_hp
	{
		std::int32_t m_current = 10;
		std::int32_t m_max = 10;
	};

	struct c_mp
	{
		std::int32_t m_current = 10;
		std::int32_t m_max = 10;
	};

	struct c_position
	{
		glm::ivec2 m_pos;
	};

	struct c_visual
	{
		screen_cell m_cell;
	};
	
	constexpr auto ACTOR_ENERGY_PER_CYCLE = std::int32_t{ 10 };
	constexpr auto ACTOR_ENERGY_PER_TURN = std::int32_t{ 100 };

	struct c_actor
	{
		void add_energy() { m_energy += ACTOR_ENERGY_PER_CYCLE; }
		bool has_turn_energy() const { return m_energy >= ACTOR_ENERGY_PER_TURN; }
		void take_turn_energy() { m_energy -= ACTOR_ENERGY_PER_TURN; }

		std::int32_t m_energy = 0;
	};

	struct c_player_tag { };
	struct c_monster_tag { };
	struct c_object_tag { };

	// PLAYER:
	
	entt::entity player_create_entity(entt::registry& registry);
	bool player_can_use_stairs(level& level, stairs_direction dir);

	// MONSTER:

	entt::entity monster_create_entity(entt::registry& registry);
	void monster_move(level& level, entt::entity monster, c_position& pos, random::rng_t& rng);

} // rog
