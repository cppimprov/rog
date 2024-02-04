#pragma once

#include "bump_result.hpp"

#include <SDL.h>

#include <cstdint>
#include <string_view>

namespace bump
{
	
	namespace sdl
	{
		
		enum subsystems : Uint32
		{
			TIMER = SDL_INIT_TIMER,
			AUDIO = SDL_INIT_AUDIO,
			VIDEO = SDL_INIT_VIDEO,
			JOYSTICK = SDL_INIT_JOYSTICK,
			HAPTIC = SDL_INIT_HAPTIC,
			GAME_CONTROLLER = SDL_INIT_GAMECONTROLLER,
			EVENTS = SDL_INIT_EVENTS,
			SENSOR = SDL_INIT_SENSOR,
			EVERYTHING = SDL_INIT_EVERYTHING,
		};

		class context
		{
		public:

			context(): m_active(false) { }
			explicit context(bool active): m_active(active) { }

			context(context const&) = delete;
			context& operator=(context const&) = delete;

			context(context&& other): m_active(other.m_active) { other.m_active = false; }
			context& operator=(context&& other) { auto temp = std::move(other); std::swap(m_active, temp.m_active); return *this; }

			~context() { quit(); }
			
			void quit();

			bool is_active() const { return m_active; }
			
		private:

			bool m_active;
		};

		result<context, std::string_view> init();
		result<context, std::string_view> init(subsystems s);
		
		result<void, std::string_view> init_subsystems(subsystems s);

		subsystems was_init();
		subsystems was_init(subsystems s);

		void quit();
		void quit_subsystems(subsystems s);

	} // sdl
	
} // bump
