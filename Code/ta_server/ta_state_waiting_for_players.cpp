#include "ta_state.hpp"

#include "ta_server.hpp"

#include <ta_net_server.hpp>
#include <ta_world.hpp>

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_input.hpp>

namespace ta
{

	bump::gamestate waiting_for_players(bump::app& app, std::unique_ptr<ta::world> world_ptr)
	{
		auto& world = *world_ptr;

		auto app_events = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();
		auto net_events = std::queue<ta::net::net_event>();
		auto game_events = std::queue<ta::net::game_event>();

		auto server = ta::net::server(4, 2, 6543);

		if (!server.m_host.is_valid())
		{
			bump::log_error("failed to create enet server!");
			return { };
		}

		while (true)
		{
			// input
			{
				app.m_input_handler.poll(app_events, input_events);

				while (!app_events.empty())
				{
					auto event = std::move(app_events.front());
					app_events.pop();

					namespace ae = bump::input::app_events;

					if (std::holds_alternative<ae::quit>(event))
						return { };	// quit
				}

				while (!input_events.empty())
				{
					auto event = std::move(input_events.front());
					input_events.pop();

					namespace ie = bump::input::input_events;

					if (std::holds_alternative<ie::keyboard_key>(event))
					{
						auto const& k = std::get<ie::keyboard_key>(event);

						using kt = bump::input::keyboard_key;

						if (k.m_key == kt::ESCAPE && k.m_value)
							return { }; // quit
					}
				}
			}

			// network
			{
				server.poll(net_events, game_events);

				while (!net_events.empty())
				{
					auto event = std::move(net_events.front());
					net_events.pop();

					namespace ne = ta::net::net_events;

					if (std::holds_alternative<ne::connect>(event))
					{
						bump::log_info("client connected!");
						spawn_player(world, std::get<ne::connect>(event), server);
						continue;
					}

					if (std::holds_alternative<ne::disconnect>(event))
					{
						bump::log_info("client disconnected!");
						despawn_player(world, std::get<ne::disconnect>(event), server);
						continue;
					}
				}

				while (!game_events.empty())
				{
					bump::log_info("game event received!");

					// note: we shouldn't be receiving any game events here!
				}

				// notify players to move to the main_loop
				if (server.m_host.get_connected_peer_count() == 4)
				{
					server.broadcast(0, net::game_events::ready{ }, ENET_PACKET_FLAG_RELIABLE);

					return { [&, world = std::move(world_ptr), server = std::move(server)] (bump::app& app) mutable { return main_loop(app, std::move(world), std::move(server)); } };
				}
			}

			render_world(app.m_window, app.m_renderer, world);
		}

		return { };
	}

} // ta