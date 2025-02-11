#pragma once

#include "bump_input.hpp"
#include "bump_sdl_gamepad.hpp"

#include <SDL.h>

#include <optional>
#include <queue>
#include <vector>

namespace bump
{
	
	namespace sdl
	{

		class window;
		
		class input_handler
		{
		public:

			explicit input_handler(window& window);

			bool is_keyboard_key_pressed(input::keyboard_key key) const;
			bool is_mouse_button_pressed(input::mouse_button button) const;
			glm::ivec2 get_mouse_position() const;

			void start_text_input();
			void stop_text_input();
			bool is_text_input_enabled();

			void poll(std::queue<input::app_event>& app_events, std::queue<input::input_event>& input_events);

		private:

			window& m_window;
			std::uint32_t m_text_input_count;
			std::vector<sdl::gamepad> m_gamepads;
		};
		
	} // sdl
	
} // bump
