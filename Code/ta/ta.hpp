
#include <bump_math.hpp>

#include <map>
#include <vector>

namespace ta
{

	enum direction { up_left, up, up_right, left, right, down_left, down, down_right, };
	enum tile_type { wall, floor, };
	enum powerup_type { player_heal, player_shield, player_speed, player_firing_frequency, bullet_bounce, bullet_damage, bullet_speed };

	using hp_t = std::int32_t;

	struct player
	{
		std::uint32_t m_id;

		hp_t m_hp;

		glm::vec3 m_position;
		direction m_direction;
		bool m_moving;

		std::map<powerup_type, float> m_powerup_timers;
	};

	struct powerup
	{
		powerup_type m_type;
		glm::vec3 m_position;
	};

	struct bullet
	{
		glm::vec3 m_position;
		direction m_direction;
		float m_speed;
		hp_t m_damage;
	};

	struct world
	{
		std::vector<player> m_players;
		std::vector<powerup> m_powerups;
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
