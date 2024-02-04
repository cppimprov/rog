#pragma once

#include "ta_direction.hpp"
#include "ta_world.hpp"

#include <bump_io.hpp>
#include <bump_enet.hpp>
#include <bump_time.hpp>

#include <cstdint>
#include <variant>

namespace ta
{

	namespace net
	{

		enum class net_event_type : std::uint8_t { CONNECT, DISCONNECT, };

		namespace net_events
		{

			struct connect    { };
			struct disconnect { };

		} // events

		using net_event = std::variant
		<
			std::monostate, // invalid event type
			net_events::connect,
			net_events::disconnect
		>;

		struct peer_net_event
		{
			bump::enet::peer m_peer;
			net_event m_event;
		};

		enum class game_event_type : std::uint8_t
		{
			SPAWN,
			DESPAWN,
			READY,
			GAME_OVER,
			INPUT,
			SPAWN_BULLET,
			DESPAWN_BULLET,
			SET_HP,
			SET_POWERUP_TIMER,
			CLEAR_POWERUP_TIMER,
			SPAWN_POWERUP,
			DESPAWN_POWERUP,
			PLAYER_DEATH
		};

		namespace game_events
		{

			struct spawn { std::uint8_t m_slot_index; bool m_self; };
			struct despawn { std::uint8_t m_slot_index; };
			struct ready { };
			struct game_over { };
			struct input { bump::high_res_duration_t m_timestamp; bool m_moving; ta::direction m_direction; bool m_firing; };
			struct spawn_bullet { std::uint8_t m_owner_slot_index; std::uint32_t m_id; glm::vec2 m_pos_px; glm::vec2 m_vel_px; };
			struct despawn_bullet { std::uint32_t m_id; };
			struct set_hp { std::uint8_t m_slot_index; std::uint32_t m_hp; };
			struct set_powerup_timer { std::uint8_t m_slot_index; enum class powerup_type m_type; float m_time; };
			struct clear_powerup_timer { std::uint8_t m_slot_index; enum class powerup_type m_type; };
			struct spawn_powerup { enum class powerup_type m_type; std::uint32_t m_id; glm::vec2 m_pos_px; };
			struct despawn_powerup { std::uint32_t m_id; };
			struct player_death { std::uint8_t m_slot_index; };

		} // events

		using game_event = std::variant
		<
			std::monostate, // invalid event type
			game_events::spawn,
			game_events::despawn,
			game_events::ready,
			game_events::game_over,
			game_events::input,
			game_events::spawn_bullet,
			game_events::despawn_bullet,
			game_events::set_hp,
			game_events::set_powerup_timer,
			game_events::clear_powerup_timer,
			game_events::spawn_powerup,
			game_events::despawn_powerup,
			game_events::player_death
		>;

		struct peer_game_event
		{
			bump::enet::peer m_peer;
			game_event m_event;
		};

	} // net

} // ta

namespace bump
{

	namespace io
	{

		template<>
		struct write_impl<std::monostate>
		{
			static void write(std::ostream& , std::monostate ) { }
		};

		template<>
		struct write_impl<ta::net::game_event_type>
		{
			static void write(std::ostream& os, ta::net::game_event_type value)
			{
				io::write(os, static_cast<std::uint8_t>(value));
			}
		};

		template<>
		struct read_impl<ta::net::game_event_type>
		{
			static ta::net::game_event_type read(std::istream& is)
			{
				return static_cast<ta::net::game_event_type>(io::read<std::uint8_t>(is));
			}
		};

		template<>
		struct write_impl<ta::direction>
		{
			static void write(std::ostream& os, ta::direction value)
			{
				io::write(os, static_cast<std::uint8_t>(value));
			}
		};

		template<>
		struct read_impl<ta::direction>
		{
			static ta::direction read(std::istream& is)
			{
				return static_cast<ta::direction>(io::read<std::uint8_t>(is));
			}
		};

		template<>
		struct write_impl<ta::powerup_type>
		{
			static void write(std::ostream& os, ta::powerup_type value)
			{
				io::write(os, static_cast<std::uint8_t>(value));
			}
		};

		template<>
		struct read_impl<ta::powerup_type>
		{
			static ta::powerup_type read(std::istream& is)
			{
				return static_cast<ta::powerup_type>(io::read<std::uint8_t>(is));
			}
		};

		template<>
		struct write_impl<ta::net::game_events::spawn>
		{
			static void write(std::ostream& os, ta::net::game_events::spawn value)
			{
				io::write(os, ta::net::game_event_type::SPAWN);
				io::write(os, value.m_slot_index);
				io::write(os, value.m_self);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::spawn>
		{
			static ta::net::game_events::spawn read(std::istream& is)
			{
				return
				{
					.m_slot_index = io::read<std::uint8_t>(is),
					.m_self       = io::read<bool>(is)
				};
			}
		};

		template<>
		struct write_impl<ta::net::game_events::despawn>
		{
			static void write(std::ostream& os, ta::net::game_events::despawn value)
			{
				io::write(os, ta::net::game_event_type::DESPAWN);
				io::write(os, value.m_slot_index);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::despawn>
		{
			static ta::net::game_events::despawn read(std::istream& is)
			{
				return { .m_slot_index = io::read<std::uint8_t>(is) };
			}
		};

		template<>
		struct write_impl<ta::net::game_events::ready>
		{
			static void write(std::ostream& os, ta::net::game_events::ready)
			{
				io::write(os, ta::net::game_event_type::READY);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::ready>
		{
			static ta::net::game_events::ready read(std::istream& )
			{
				return { };
			}
		};

		template<>
		struct write_impl<ta::net::game_events::game_over>
		{
			static void write(std::ostream& os, ta::net::game_events::game_over)
			{
				io::write(os, ta::net::game_event_type::GAME_OVER);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::game_over>
		{
			static ta::net::game_events::game_over read(std::istream& )
			{
				return { };
			}
		};

		template<>
		struct write_impl<ta::net::game_events::input>
		{
			static void write(std::ostream& os, ta::net::game_events::input value)
			{
				io::write(os, ta::net::game_event_type::INPUT);
				io::write(os, value.m_timestamp);
				io::write(os, value.m_moving);
				io::write(os, value.m_direction);
				io::write(os, value.m_firing);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::input>
		{
			static ta::net::game_events::input read(std::istream& is)
			{
				return
				{
					.m_timestamp = io::read<bump::high_res_duration_t>(is),
					.m_moving    = io::read<bool>(is),
					.m_direction = io::read<ta::direction>(is),
					.m_firing    = io::read<bool>(is)
				};
			}
		};

		template<>
		struct write_impl<ta::net::game_events::spawn_bullet>
		{
			static void write(std::ostream& os, ta::net::game_events::spawn_bullet value)
			{
				io::write(os, ta::net::game_event_type::SPAWN_BULLET);
				io::write(os, value.m_owner_slot_index);
				io::write(os, value.m_id);
				io::write(os, value.m_pos_px);
				io::write(os, value.m_vel_px);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::spawn_bullet>
		{
			static ta::net::game_events::spawn_bullet read(std::istream& is)
			{
				return
				{
					.m_owner_slot_index = io::read<std::uint8_t>(is),
					.m_id               = io::read<std::uint32_t>(is),
					.m_pos_px           = io::read<glm::vec2>(is),
					.m_vel_px           = io::read<glm::vec2>(is)
				};
			}
		};

		template<>
		struct write_impl<ta::net::game_events::despawn_bullet>
		{
			static void write(std::ostream& os, ta::net::game_events::despawn_bullet value)
			{
				io::write(os, ta::net::game_event_type::DESPAWN_BULLET);
				io::write(os, value.m_id);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::despawn_bullet>
		{
			static ta::net::game_events::despawn_bullet read(std::istream& is)
			{
				return { .m_id = io::read<std::uint32_t>(is) };
			}
		};

		template<>
		struct write_impl<ta::net::game_events::set_hp>
		{
			static void write(std::ostream& os, ta::net::game_events::set_hp value)
			{
				io::write(os, ta::net::game_event_type::SET_HP);
				io::write(os, value.m_slot_index);
				io::write(os, value.m_hp);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::set_hp>
		{
			static ta::net::game_events::set_hp read(std::istream& is)
			{
				return
				{
					.m_slot_index = io::read<std::uint8_t>(is),
					.m_hp         = io::read<std::uint32_t>(is)
				};
			}
		};

		template<>
		struct write_impl<ta::net::game_events::set_powerup_timer>
		{
			static void write(std::ostream& os, ta::net::game_events::set_powerup_timer value)
			{
				io::write(os, ta::net::game_event_type::SET_POWERUP_TIMER);
				io::write(os, value.m_slot_index);
				io::write(os, value.m_type);
				io::write(os, value.m_time);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::set_powerup_timer>
		{
			static ta::net::game_events::set_powerup_timer read(std::istream& is)
			{
				return
				{
					.m_slot_index = io::read<std::uint8_t>(is),
					.m_type       = io::read<ta::powerup_type>(is),
					.m_time       = io::read<float>(is)
				};
			}
		};

		template<>
		struct write_impl<ta::net::game_events::clear_powerup_timer>
		{
			static void write(std::ostream& os, ta::net::game_events::clear_powerup_timer value)
			{
				io::write(os, ta::net::game_event_type::CLEAR_POWERUP_TIMER);
				io::write(os, value.m_slot_index);
				io::write(os, value.m_type);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::clear_powerup_timer>
		{
			static ta::net::game_events::clear_powerup_timer read(std::istream& is)
			{
				return
				{
					.m_slot_index = io::read<std::uint8_t>(is),
					.m_type       = io::read<ta::powerup_type>(is)
				};
			}
		};

		template<>
		struct write_impl<ta::net::game_events::spawn_powerup>
		{
			static void write(std::ostream& os, ta::net::game_events::spawn_powerup value)
			{
				io::write(os, ta::net::game_event_type::SPAWN_POWERUP);
				io::write(os, value.m_type);
				io::write(os, value.m_id);
				io::write(os, value.m_pos_px);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::spawn_powerup>
		{
			static ta::net::game_events::spawn_powerup read(std::istream& is)
			{
				return
				{
					.m_type   = io::read<ta::powerup_type>(is),
					.m_id     = io::read<std::uint32_t>(is),
					.m_pos_px = io::read<glm::vec2>(is)
				};
			}
		};

		template<>
		struct write_impl<ta::net::game_events::despawn_powerup>
		{
			static void write(std::ostream& os, ta::net::game_events::despawn_powerup value)
			{
				io::write(os, ta::net::game_event_type::DESPAWN_POWERUP);
				io::write(os, value.m_id);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::despawn_powerup>
		{
			static ta::net::game_events::despawn_powerup read(std::istream& is)
			{
				return { .m_id = io::read<std::uint32_t>(is) };
			}
		};

		template<>
		struct write_impl<ta::net::game_events::player_death>
		{
			static void write(std::ostream& os, ta::net::game_events::player_death value)
			{
				io::write(os, ta::net::game_event_type::PLAYER_DEATH);
				io::write(os, value.m_slot_index);
			}
		};

		template<>
		struct read_impl<ta::net::game_events::player_death>
		{
			static ta::net::game_events::player_death read(std::istream& is)
			{
				return { .m_slot_index = io::read<std::uint8_t>(is) };
			}
		};

		template<>
		struct write_impl<ta::net::game_event>
		{
			static void write(std::ostream& os, ta::net::game_event const& value)
			{
				std::visit([&] (auto const& v) { io::write(os, v); }, value);
			}
		};

		template<>
		struct read_impl<ta::net::game_event>
		{
			static ta::net::game_event read(std::istream& is)
			{
				using event_type = ta::net::game_event_type;
				namespace ge = ta::net::game_events;

				auto const type = io::read<event_type>(is);
				
				switch (type)
				{
				case event_type::SPAWN              : return { io::read<ge::spawn>(is) };
				case event_type::DESPAWN            : return { io::read<ge::despawn>(is) };
				case event_type::READY              : return { io::read<ge::ready>(is) };
				case event_type::GAME_OVER          : return { io::read<ge::game_over>(is) };
				case event_type::INPUT              : return { io::read<ge::input>(is) };
				case event_type::SPAWN_BULLET       : return { io::read<ge::spawn_bullet>(is) };
				case event_type::DESPAWN_BULLET     : return { io::read<ge::despawn_bullet>(is) };
				case event_type::SET_HP             : return { io::read<ge::set_hp>(is) };
				case event_type::SET_POWERUP_TIMER  : return { io::read<ge::set_powerup_timer>(is) };
				case event_type::CLEAR_POWERUP_TIMER: return { io::read<ge::clear_powerup_timer>(is) };
				case event_type::SPAWN_POWERUP      : return { io::read<ge::spawn_powerup>(is) };
				case event_type::DESPAWN_POWERUP    : return { io::read<ge::despawn_powerup>(is) };
				case event_type::PLAYER_DEATH       : return { io::read<ge::player_death>(is) };
				}

				return { };
			}
		};
	
	} // io

} // bump
