#pragma once

#include <bump_die.hpp>
#include <bump_math.hpp>

#include <array>
#include <map>
#include <optional>
#include <vector>

namespace ta
{

	enum direction { up_left, up, up_right, left, right, down_left, down, down_right, };
	enum powerup_type { player_heal, player_shield, player_speed, player_firing_frequency, bullet_bounce, bullet_damage, bullet_speed };

	auto constexpr direction_vectors = std::array<glm::vec2, 9>
	{
		glm::vec2{ -1.f,  1.f },
		glm::vec2{  0.f,  1.f },
		glm::vec2{  1.f,  1.f },
		glm::vec2{ -1.f,  0.f },
		glm::vec2{  1.f,  0.f },
		glm::vec2{ -1.f, -1.f },
		glm::vec2{  0.f, -1.f },
		glm::vec2{  1.f, -1.f },
	};

	inline glm::vec2 dir_to_vec(direction dir)
	{
		return direction_vectors[static_cast<std::size_t>(dir)];
	}

	inline direction reflect_dir(direction dir)
	{
		switch (dir)
		{
		case direction::up_left:    return direction::down_right;
		case direction::up:         return direction::down;
		case direction::up_right:   return direction::down_left;
		case direction::left:       return direction::right;
		case direction::right:      return direction::left;
		case direction::down_left:  return direction::up_right;
		case direction::down:       return direction::up;
		case direction::down_right: return direction::up_left;
		}
		bump::die();
	}

	inline std::optional<direction> get_input_dir(bool up, bool down, bool left, bool right)
	{
		if (up)
		{
			if (left) return direction::up_left;
			if (right) return direction::up_right;
			return direction::up;
		}

		if (down)
		{
			if (left) return direction::down_left;
			if (right) return direction::down_right;
			return direction::down;
		}

		if (left) return direction::left;
		if (right) return direction::right;
		
		return { };
	}

	using hp_t = std::int32_t;

	struct player
	{
		std::uint32_t m_id;
		hp_t m_hp;
		glm::vec2 m_position;
		direction m_direction;
		bool m_moving;
		glm::vec3 m_color;
		std::map<powerup_type, float> m_powerup_timers;
	};

	struct bullet
	{
		std::uint32_t m_owner_id;
		glm::vec2 m_position;
		direction m_direction;
		glm::vec3 m_color;
		float m_speed;
		hp_t m_damage;
		float m_lifetime;
	};

	struct powerup
	{
		powerup_type m_type;
		glm::vec2 m_position;
		glm::vec3 m_color;
		float m_lifetime;
	};

	struct bounds
	{
		glm::vec2 m_min;
		glm::vec2 m_max;

		bool is_valid() const
		{
			return m_min.x <= m_max.x && m_min.y <= m_max.y;
		}
	};

	struct world
	{
		bounds m_bounds;

		std::vector<player> m_players;
		std::vector<powerup> m_powerups;
		std::vector<bullet> m_bullets;
	};

	namespace net
	{

		enum class message_type : std::uint8_t { HELLO, GOODBYE, };

		using message_id_t = std::uint32_t;

		struct message
		{
			message_type m_type;
			message_id_t m_id;
			std::vector<std::uint8_t> m_data;
		};

	} // net

} // ta
