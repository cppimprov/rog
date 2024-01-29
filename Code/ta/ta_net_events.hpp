#pragma once

#include "ta_direction.hpp"

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

		enum class game_event_type : std::uint8_t { SPAWN, DESPAWN, READY, INPUT, SPAWN_BULLET, DESPAWN_BULLET, FIRE, SET_HP, };

		namespace game_events
		{

			struct spawn   { std::uint8_t m_slot_index; bool m_self; };
			struct despawn { std::uint8_t m_slot_index; };
			struct ready   { };
			struct input   { bump::high_res_duration_t m_timestamp; bool m_moving; ta::direction m_direction; bool m_firing; };
			struct spawn_bullet   { std::uint8_t m_owner_slot_index; std::uint32_t m_id; glm::vec2 m_pos_px; glm::vec2 m_vel_px; };
			struct despawn_bullet { std::uint32_t m_id; };
			struct set_hp  { std::uint8_t m_slot_index; std::uint32_t m_hp; };

		} // events

		using game_event = std::variant
		<
			std::monostate, // invalid event type
			game_events::spawn,
			game_events::despawn,
			game_events::ready,
			game_events::input,
			game_events::spawn_bullet,
			game_events::despawn_bullet,
			game_events::set_hp
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
				auto const slot_index = io::read<std::uint8_t>(is);
				auto const self = io::read<bool>(is);
				return { slot_index, self };
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
				auto const slot_index = io::read<std::uint8_t>(is);
				return { slot_index };
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
				auto const timestamp = io::read<bump::high_res_duration_t>(is);
				auto const moving = io::read<bool>(is);
				auto const direction = io::read<ta::direction>(is);
				auto const firing = io::read<bool>(is);
				return { timestamp, moving, direction, firing };
			}
		};

		template<>
		struct write_impl<ta::net::game_events::spawn_bullet>
		{
			static void write(std::ostream& os, ta::net::game_events::spawn_bullet value)
			{
				io::write(os, ta::net::game_event_type::FIRE);
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
				auto const owner_slot_index = io::read<std::uint8_t>(is);
				auto const id = io::read<std::uint32_t>(is);
				auto const pos_px = io::read<glm::vec2>(is);
				auto const vel_px = io::read<glm::vec2>(is);
				return { owner_slot_index, id, pos_px, vel_px };
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
				auto const id = io::read<std::uint32_t>(is);
				return { id };
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
				auto const slot_index = io::read<std::uint8_t>(is);
				auto const hp = io::read<std::uint32_t>(is);
				return { slot_index, hp };
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
				auto const type = io::read<ta::net::game_event_type>(is);
				
				switch (type)
				{
				case ta::net::game_event_type::SPAWN: return { io::read<ta::net::game_events::spawn>(is) };
				case ta::net::game_event_type::DESPAWN: return { io::read<ta::net::game_events::despawn>(is) };
				case ta::net::game_event_type::READY: return { io::read<ta::net::game_events::ready>(is) };
				case ta::net::game_event_type::INPUT: return { io::read<ta::net::game_events::input>(is) };
				case ta::net::game_event_type::FIRE: return { io::read<ta::net::game_events::spawn_bullet>(is) };
				case ta::net::game_event_type::DESPAWN_BULLET: return { io::read<ta::net::game_events::despawn_bullet>(is) };
				case ta::net::game_event_type::SET_HP: return { io::read<ta::net::game_events::set_hp>(is) };
				}

				return { };
			}
		};
	
	} // io

} // bump
