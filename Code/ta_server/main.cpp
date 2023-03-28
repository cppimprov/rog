
#include <bump_log.hpp>
#include <bump_net.hpp>
#include <bump_math.hpp>

#include <iostream>
#include <map>
#include <string>
#include <system_error>

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

#include <iostream>

int main()
{
	using namespace bump;

	auto const net_context = net::init_context().unwrap();
	auto const local_endpoint = net::get_address_info_loopback(net::ip_address_family::V6, net::ip_protocol::UDP, 4376).unwrap().m_endpoints.front();
	auto udp_socket = net::make_udp_socket(local_endpoint, net::blocking_mode::NON_BLOCKING).unwrap();
	auto udp_read_buffer = std::vector<std::uint8_t>(128, '\0');
	auto client_endpoints = std::vector<net::endpoint>();

	while (true)
	{
		// check messages...
		{
			while (true)
			{
				if (!udp_socket.is_open())
				{
					log_error("udp_socket closed unexpectedly.");
					break;
				}

				auto const recv_result = udp_socket.receive_from(std::span<std::uint8_t>(udp_read_buffer.data(), udp_read_buffer.size()));

				if (!recv_result.has_value())
				{
					auto const e = recv_result.error();
					log_error("udp_socket receive() error: " + std::to_string(e.code().value()) + " " + std::string(e.what()));
					continue;
				}

				auto const [remote, msg_size] = recv_result.value();

				if (auto const c = std::find(client_endpoints.begin(), client_endpoints.end(), remote); c != client_endpoints.end()) // existing client
				{
					if (msg_size == 0)
						continue; // invalid size...

					auto const type = static_cast<ta::net::message_type>(udp_read_buffer[0]);

					if (type == ta::net::message_type::GOODBYE)
					{
						client_endpoints.erase(c);
						// TODO: update other clients
					}
					else
					{
						continue; // invalid message...
					}
				}
				else  // new client
				{
					if (msg_size != 1)
						continue; // wrong size...

					if (udp_read_buffer[0] != static_cast<std::uint8_t>(ta::net::message_type::HELLO))
						continue; // wrong message...

					log_info("client connected");

					// ok! new client hello -> add client to endpoint list
					client_endpoints.push_back(remote);

					// TODO: update new client with world state
					// TODO: inform other clients about new player
				}
			}
		}

		// ... check for new clients
		// ... handle connections (send game state to new clients)
		// ... handle disconnections
		// ... receive game data
		// ... update game data
		// ... send game state
	}

	std::cout << "done!" << std::endl;
}


// TODO:
	// create ta_client program.
	// add connection / disconnection to that program.
	// maybe we need to get the basic rules working, and then worry about how to go about replicating the state across the network?

	// make it work on WSL!