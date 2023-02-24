
#include <bump_log.hpp>
#include <bump_net.hpp>
#include <bump_math.hpp>

#include <map>

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

} // ta

#include <iostream>

int main()
{
	using namespace bump;

	auto const context = net::init_context().unwrap();
	auto const endpoint = net::get_address_info_loopback(net::ip_address_family::V6, net::ip_protocol::UDP, 4376).unwrap();
	
	// ok... so we want a udp socket
	// ...

	while (true)
	{

		// ... check for new clients
		// ... handle connections (send game state to new clients)
		// ... handle disconnections
		// ... receive game data
		// ... update game data
		// ... send game state
	}

	std::cout << "done!" << std::endl;
}

// todo next:
	// commit!
	// continue... !