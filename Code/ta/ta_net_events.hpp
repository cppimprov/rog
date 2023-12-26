#pragma once

#include "bump_io.hpp"

#include <cstdint>
#include <variant>

namespace ta
{

	namespace net
	{

		enum class event_type : std::uint8_t { SPAWN, DESPAWN, READY, };

		namespace events
		{

			struct spawn   { std::uint8_t m_slot_index; bool m_self; };
			struct despawn { std::uint8_t m_slot_index; };
			struct ready   { };

		} // events

		using event = std::variant
		<
			std::monostate, // invalid event type
			events::spawn,
			events::despawn,
			events::ready
		>;

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
		struct write_impl<ta::net::event_type>
		{
			static void write(std::ostream& os, ta::net::event_type value)
			{
				io::write(os, static_cast<std::uint8_t>(value));
			}
		};

		template<>
		struct read_impl<ta::net::event_type>
		{
			static ta::net::event_type read(std::istream& is)
			{
				return static_cast<ta::net::event_type>(io::read<std::uint8_t>(is));
			}
		};

		template<>
		struct write_impl<ta::net::events::spawn>
		{
			static void write(std::ostream& os, ta::net::events::spawn value)
			{
				io::write(os, ta::net::event_type::SPAWN);
				io::write(os, value.m_slot_index);
				io::write(os, value.m_self);
			}
		};

		template<>
		struct read_impl<ta::net::events::spawn>
		{
			static ta::net::events::spawn read(std::istream& is)
			{
				auto const slot_index = io::read<std::uint8_t>(is);
				auto const self = io::read<bool>(is);
				return { slot_index, self };
			}
		};

		template<>
		struct write_impl<ta::net::events::despawn>
		{
			static void write(std::ostream& os, ta::net::events::despawn value)
			{
				io::write(os, ta::net::event_type::DESPAWN);
				io::write(os, value.m_slot_index);
			}
		};

		template<>
		struct read_impl<ta::net::events::despawn>
		{
			static ta::net::events::despawn read(std::istream& is)
			{
				auto const slot_index = io::read<std::uint8_t>(is);
				return { slot_index };
			}
		};

		template<>
		struct write_impl<ta::net::events::ready>
		{
			static void write(std::ostream& os, ta::net::events::ready)
			{
				io::write(os, ta::net::event_type::READY);
			}
		};

		template<>
		struct read_impl<ta::net::events::ready>
		{
			static ta::net::events::ready read(std::istream& )
			{
				return { };
			}
		};
	
		template<>
		struct write_impl<ta::net::event>
		{
			static void write(std::ostream& os, ta::net::event const& value)
			{
				std::visit([&] (auto const& v) { io::write(os, v); }, value);
			}
		};

		template<>
		struct read_impl<ta::net::event>
		{
			static ta::net::event read(std::istream& is)
			{
				auto const type = io::read<ta::net::event_type>(is);
				
				switch (type)
				{
				case ta::net::event_type::SPAWN: return { io::read<ta::net::events::spawn>(is) };
				case ta::net::event_type::DESPAWN: return { io::read<ta::net::events::despawn>(is) };
				case ta::net::event_type::READY: return { io::read<ta::net::events::ready>(is) };
				}

				return { };
			}
		};
	
	} // io

} // bump
