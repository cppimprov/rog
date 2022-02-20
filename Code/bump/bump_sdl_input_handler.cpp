#include "bump_sdl_input_handler.hpp"

#include "bump_pair_map.hpp"
#include "bump_sdl_window.hpp"

#include <algorithm>

namespace bump
{
	
	namespace sdl
	{

		namespace
		{

			using sdl_scancode_pair_type = std::pair<SDL_Scancode, input::keyboard_key>;

			auto const sdl_scancodes = pair_map<SDL_Scancode, input::keyboard_key>
			{
				sdl_scancode_pair_type{ SDL_SCANCODE_A, input::keyboard_key::A },
				sdl_scancode_pair_type{ SDL_SCANCODE_B, input::keyboard_key::B },
				sdl_scancode_pair_type{ SDL_SCANCODE_C, input::keyboard_key::C },
				sdl_scancode_pair_type{ SDL_SCANCODE_D, input::keyboard_key::D },
				sdl_scancode_pair_type{ SDL_SCANCODE_E, input::keyboard_key::E },
				sdl_scancode_pair_type{ SDL_SCANCODE_F, input::keyboard_key::F },
				sdl_scancode_pair_type{ SDL_SCANCODE_G, input::keyboard_key::G },
				sdl_scancode_pair_type{ SDL_SCANCODE_H, input::keyboard_key::H },
				sdl_scancode_pair_type{ SDL_SCANCODE_I, input::keyboard_key::I },
				sdl_scancode_pair_type{ SDL_SCANCODE_J, input::keyboard_key::J },
				sdl_scancode_pair_type{ SDL_SCANCODE_K, input::keyboard_key::K },
				sdl_scancode_pair_type{ SDL_SCANCODE_L, input::keyboard_key::L },
				sdl_scancode_pair_type{ SDL_SCANCODE_M, input::keyboard_key::M },
				sdl_scancode_pair_type{ SDL_SCANCODE_N, input::keyboard_key::N },
				sdl_scancode_pair_type{ SDL_SCANCODE_O, input::keyboard_key::O },
				sdl_scancode_pair_type{ SDL_SCANCODE_P, input::keyboard_key::P },
				sdl_scancode_pair_type{ SDL_SCANCODE_Q, input::keyboard_key::Q },
				sdl_scancode_pair_type{ SDL_SCANCODE_R, input::keyboard_key::R },
				sdl_scancode_pair_type{ SDL_SCANCODE_S, input::keyboard_key::S },
				sdl_scancode_pair_type{ SDL_SCANCODE_T, input::keyboard_key::T },
				sdl_scancode_pair_type{ SDL_SCANCODE_U, input::keyboard_key::U },
				sdl_scancode_pair_type{ SDL_SCANCODE_V, input::keyboard_key::V },
				sdl_scancode_pair_type{ SDL_SCANCODE_W, input::keyboard_key::W },
				sdl_scancode_pair_type{ SDL_SCANCODE_X, input::keyboard_key::X },
				sdl_scancode_pair_type{ SDL_SCANCODE_Y, input::keyboard_key::Y },
				sdl_scancode_pair_type{ SDL_SCANCODE_Z, input::keyboard_key::Z },

				sdl_scancode_pair_type{ SDL_SCANCODE_1, input::keyboard_key::N1 },
				sdl_scancode_pair_type{ SDL_SCANCODE_2, input::keyboard_key::N2 },
				sdl_scancode_pair_type{ SDL_SCANCODE_3, input::keyboard_key::N3 },
				sdl_scancode_pair_type{ SDL_SCANCODE_4, input::keyboard_key::N4 },
				sdl_scancode_pair_type{ SDL_SCANCODE_5, input::keyboard_key::N5 },
				sdl_scancode_pair_type{ SDL_SCANCODE_6, input::keyboard_key::N6 },
				sdl_scancode_pair_type{ SDL_SCANCODE_7, input::keyboard_key::N7 },
				sdl_scancode_pair_type{ SDL_SCANCODE_8, input::keyboard_key::N8 },
				sdl_scancode_pair_type{ SDL_SCANCODE_9, input::keyboard_key::N9 },
				sdl_scancode_pair_type{ SDL_SCANCODE_0, input::keyboard_key::N0 },

				sdl_scancode_pair_type{ SDL_SCANCODE_F1, input::keyboard_key::F1 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F2, input::keyboard_key::F2 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F3, input::keyboard_key::F3 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F4, input::keyboard_key::F4 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F5, input::keyboard_key::F5 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F6, input::keyboard_key::F6 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F7, input::keyboard_key::F7 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F8, input::keyboard_key::F8 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F9, input::keyboard_key::F9 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F10, input::keyboard_key::F10 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F11, input::keyboard_key::F11 },
				sdl_scancode_pair_type{ SDL_SCANCODE_F12, input::keyboard_key::F12 },

				sdl_scancode_pair_type{ SDL_SCANCODE_ESCAPE, input::keyboard_key::ESCAPE },

				sdl_scancode_pair_type{ SDL_SCANCODE_PRINTSCREEN, input::keyboard_key::PRINTSCREEN },
				sdl_scancode_pair_type{ SDL_SCANCODE_SCROLLLOCK, input::keyboard_key::SCROLLLOCK },
				sdl_scancode_pair_type{ SDL_SCANCODE_PAUSE, input::keyboard_key::PAUSE },

				sdl_scancode_pair_type{ SDL_SCANCODE_GRAVE, input::keyboard_key::BACKTICK },
				sdl_scancode_pair_type{ SDL_SCANCODE_MINUS, input::keyboard_key::MINUS },
				sdl_scancode_pair_type{ SDL_SCANCODE_EQUALS, input::keyboard_key::EQUALS },
				sdl_scancode_pair_type{ SDL_SCANCODE_LEFTBRACKET, input::keyboard_key::LEFTSQUAREBRACKET },
				sdl_scancode_pair_type{ SDL_SCANCODE_RIGHTBRACKET, input::keyboard_key::RIGHTSQUAREBRACKET },
				sdl_scancode_pair_type{ SDL_SCANCODE_SEMICOLON, input::keyboard_key::SEMICOLON },
				sdl_scancode_pair_type{ SDL_SCANCODE_APOSTROPHE, input::keyboard_key::SINGLEQUOTE },
				sdl_scancode_pair_type{ SDL_SCANCODE_BACKSLASH, input::keyboard_key::HASH },
				sdl_scancode_pair_type{ SDL_SCANCODE_NONUSBACKSLASH, input::keyboard_key::BACKSLASH },
				sdl_scancode_pair_type{ SDL_SCANCODE_COMMA, input::keyboard_key::COMMA },
				sdl_scancode_pair_type{ SDL_SCANCODE_PERIOD, input::keyboard_key::DOT },
				sdl_scancode_pair_type{ SDL_SCANCODE_SLASH, input::keyboard_key::FORWARDSLASH },

				sdl_scancode_pair_type{ SDL_SCANCODE_BACKSPACE, input::keyboard_key::BACKSPACE },
				sdl_scancode_pair_type{ SDL_SCANCODE_RETURN, input::keyboard_key::RETURN },
				sdl_scancode_pair_type{ SDL_SCANCODE_TAB, input::keyboard_key::TAB },
				sdl_scancode_pair_type{ SDL_SCANCODE_CAPSLOCK, input::keyboard_key::CAPSLOCK },
				sdl_scancode_pair_type{ SDL_SCANCODE_LSHIFT, input::keyboard_key::LEFTSHIFT },
				sdl_scancode_pair_type{ SDL_SCANCODE_RSHIFT, input::keyboard_key::RIGHTSHIFT },
				sdl_scancode_pair_type{ SDL_SCANCODE_LCTRL, input::keyboard_key::LEFTCTRL },
				sdl_scancode_pair_type{ SDL_SCANCODE_RCTRL, input::keyboard_key::RIGHTCTRL },
				sdl_scancode_pair_type{ SDL_SCANCODE_LALT, input::keyboard_key::LEFTALT },
				sdl_scancode_pair_type{ SDL_SCANCODE_RALT, input::keyboard_key::RIGHTALT },
				sdl_scancode_pair_type{ SDL_SCANCODE_SPACE, input::keyboard_key::SPACE },

				sdl_scancode_pair_type{ SDL_SCANCODE_LGUI, input::keyboard_key::LEFTWINDOWS },
				sdl_scancode_pair_type{ SDL_SCANCODE_RGUI, input::keyboard_key::RIGHTWINDOWS },
				sdl_scancode_pair_type{ SDL_SCANCODE_MENU, input::keyboard_key::CONTEXTMENU },

				sdl_scancode_pair_type{ SDL_SCANCODE_INSERT, input::keyboard_key::INSERT },
				sdl_scancode_pair_type{ SDL_SCANCODE_DELETE, input::keyboard_key::DELETE },
				sdl_scancode_pair_type{ SDL_SCANCODE_HOME, input::keyboard_key::HOME },
				sdl_scancode_pair_type{ SDL_SCANCODE_END, input::keyboard_key::END },
				sdl_scancode_pair_type{ SDL_SCANCODE_PAGEUP, input::keyboard_key::PAGEUP },
				sdl_scancode_pair_type{ SDL_SCANCODE_PAGEDOWN, input::keyboard_key::PAGEDOWN },

				sdl_scancode_pair_type{ SDL_SCANCODE_NUMLOCKCLEAR, input::keyboard_key::NUMLOCK },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_DIVIDE, input::keyboard_key::NUMDIVIDE },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_MULTIPLY, input::keyboard_key::NUMMULTIPLY },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_MINUS, input::keyboard_key::NUMMINUS },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_PLUS, input::keyboard_key::NUMPLUS },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_ENTER, input::keyboard_key::NUMENTER },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_PERIOD, input::keyboard_key::NUMDOT },

				sdl_scancode_pair_type{ SDL_SCANCODE_KP_0, input::keyboard_key::NUM0 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_1, input::keyboard_key::NUM1 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_2, input::keyboard_key::NUM2 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_3, input::keyboard_key::NUM3 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_4, input::keyboard_key::NUM4 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_5, input::keyboard_key::NUM5 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_6, input::keyboard_key::NUM6 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_7, input::keyboard_key::NUM7 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_8, input::keyboard_key::NUM8 },
				sdl_scancode_pair_type{ SDL_SCANCODE_KP_9, input::keyboard_key::NUM9 },

				sdl_scancode_pair_type{ SDL_SCANCODE_LEFT, input::keyboard_key::ARROWLEFT },
				sdl_scancode_pair_type{ SDL_SCANCODE_RIGHT, input::keyboard_key::ARROWRIGHT },
				sdl_scancode_pair_type{ SDL_SCANCODE_UP, input::keyboard_key::ARROWUP },
				sdl_scancode_pair_type{ SDL_SCANCODE_DOWN, input::keyboard_key::ARROWDOWN },
			};

			input::keyboard_key sdl_scancode_to_keyboard_key(SDL_Scancode scancode)
			{
				auto entry = sdl_scancodes.find_second(scancode);

				if (!entry)
					return input::keyboard_key::UNRECOGNISED;
				
				return entry.value();
			}

			SDL_Scancode keyboard_key_to_sdl_scancode(input::keyboard_key key)
			{
				auto entry = sdl_scancodes.find_first(key);

				die_if(!entry);

				return entry.value();
			}

			using sdl_mousebutton_pair_type = std::pair<Uint8, input::mouse_button>;

			auto const sdl_mousebuttons = pair_map<Uint8, input::mouse_button>
			{
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_LEFT }, input::mouse_button::LEFT },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_MIDDLE }, input::mouse_button::MIDDLE },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_RIGHT }, input::mouse_button::RIGHT },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X1 }, input::mouse_button::X1 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 }, input::mouse_button::X2 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 1 }, input::mouse_button::X3 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 2 }, input::mouse_button::X4 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 3 }, input::mouse_button::X5 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 4 }, input::mouse_button::X5 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 5 }, input::mouse_button::X6 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 6 }, input::mouse_button::X7 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 7 }, input::mouse_button::X8 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 8 }, input::mouse_button::X9 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 9 }, input::mouse_button::X10 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 10 }, input::mouse_button::X11 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 11 }, input::mouse_button::X12 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 12 }, input::mouse_button::X13 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 13 }, input::mouse_button::X14 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 14 }, input::mouse_button::X15 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 15 }, input::mouse_button::X16 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 16 }, input::mouse_button::X17 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 17 }, input::mouse_button::X18 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 18 }, input::mouse_button::X19 },
				sdl_mousebutton_pair_type{ Uint8{ SDL_BUTTON_X2 + 19 }, input::mouse_button::X20 },
			};

			input::mouse_button sdl_mousebutton_to_mouse_button(Uint8 button)
			{
				return sdl_mousebuttons.get_second(button);
			}

			Uint8 mouse_button_to_sdl_mousebutton(input::mouse_button button)
			{
				return sdl_mousebuttons.get_first(button);
			}

			using sdl_controllerbutton_pair_type = std::pair<Uint8, input::gamepad_button>;
			
			auto const sdl_controllerbutton_control_ids = pair_map<Uint8, input::gamepad_button>
			{
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_A }, input::gamepad_button::A },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_B }, input::gamepad_button::B },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_X }, input::gamepad_button::X },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_Y }, input::gamepad_button::Y },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_BACK }, input::gamepad_button::BACK },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_GUIDE }, input::gamepad_button::GUIDE },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_START }, input::gamepad_button::START },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_LEFTSTICK }, input::gamepad_button::LEFTSTICK },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_RIGHTSTICK }, input::gamepad_button::RIGHTSTICK },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER }, input::gamepad_button::LEFTBUMPER },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER }, input::gamepad_button::RIGHTBUMPER },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_DPAD_UP }, input::gamepad_button::DPADUP },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_DPAD_DOWN }, input::gamepad_button::DPADDOWN },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_DPAD_LEFT }, input::gamepad_button::DPADLEFT },
				sdl_controllerbutton_pair_type{ Uint8{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT }, input::gamepad_button::DPADRIGHT },
			};

			input::gamepad_button sdl_controllerbutton_to_gamepad_button(Uint8 button)
			{
				return sdl_controllerbutton_control_ids.get_second(button);
			}

			using sdl_controlleraxis_pair_type = std::pair<Uint8, input::gamepad_axis>;
			
			auto const sdl_controlleraxis_control_ids = pair_map<Uint8, input::gamepad_axis>
			{
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_LEFTX }, input::gamepad_axis::STICK_LEFTX },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_LEFTY }, input::gamepad_axis::STICK_LEFTY },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_RIGHTX }, input::gamepad_axis::STICK_RIGHTX },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_RIGHTY }, input::gamepad_axis::STICK_RIGHTY },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_TRIGGERLEFT }, input::gamepad_axis::TRIGGER_LEFT },
				sdl_controlleraxis_pair_type{ Uint8{ SDL_CONTROLLER_AXIS_TRIGGERRIGHT }, input::gamepad_axis::TRIGGER_RIGHT },
			};
			
			input::gamepad_axis sdl_controlleraxis_to_gamepad_axis(Uint8 button)
			{
				return sdl_controlleraxis_control_ids.get_second(button);
			}

			float sdl_controller_axis_value_to_float(Sint16 value)
			{
				return (value < 0 ? static_cast<float>(value) / 32768 : static_cast<float>(value) / 32767);
			}
			
			input::button_state sdl_buttonstate_to_button_state(Uint8 state)
			{
				return (state == SDL_PRESSED ? input::button_state::DOWN : input::button_state::UP);
			}

		} // unnamed


		input_handler::input_handler(window& window):
			m_window(window) { }
			
		input::button_state input_handler::get_keyboard_key_state(input::keyboard_key key) const
		{
			auto const scancode = keyboard_key_to_sdl_scancode(key);
			auto const states = SDL_GetKeyboardState(nullptr);
			return sdl_buttonstate_to_button_state(states[scancode]);
		}

		input::button_state input_handler::get_mouse_button_state(input::mouse_button button) const
		{
			auto const sdl_button = mouse_button_to_sdl_mousebutton(button);
			auto const states = SDL_GetMouseState(nullptr, nullptr);
			return (states & sdl_button) ? input::button_state::DOWN : input::button_state::UP;
		}
		
		glm::ivec2 input_handler::get_mouse_position() const
		{
			auto pos = glm::ivec2();
			(void)SDL_GetMouseState(&pos.x, &pos.y);
			return pos;
		}
		
		void input_handler::poll(std::queue<input::input_event>& input_events, std::queue<input::app_event>& app_events)
		{
			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				// quit
				if (e.type == SDL_QUIT)
				{
					app_events.emplace(input::app_events::quit{ });

					continue;
				}

				// toggle fullscreen
				if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && (e.key.keysym.mod & KMOD_LALT) != 0)
				{
					auto mode = m_window.get_display_mode();
					using display_mode = sdl::window::display_mode;
					
					if (mode != display_mode::FULLSCREEN)
						m_window.set_display_mode(mode == display_mode::BORDERLESS_WINDOWED ? display_mode::WINDOWED : display_mode::BORDERLESS_WINDOWED);
					
					continue;
				}
				
				// window focus
				if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
				{
					app_events.emplace(input::app_events::pause{ true });

					continue;
				}
				if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
				{
					app_events.emplace(input::app_events::pause{ false });

					continue;
				}

				// window resized
				if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					app_events.emplace(input::app_events::resize{ { e.window.data1, e.window.data2 } });

					continue;
				}

				// controller connection / disconnection
				if (e.type == SDL_CONTROLLERDEVICEADDED)
				{
					m_gamepads.push_back(sdl::gamepad(e.cdevice.which));

					continue;
				}
				if (e.type == SDL_CONTROLLERDEVICEREMOVED)
				{
					auto entry = std::find_if(m_gamepads.begin(), m_gamepads.end(),
						[&] (sdl::gamepad const& p) { return p.get_joystick_id() == e.cdevice.which; });
					
					die_if(entry == m_gamepads.end());

					m_gamepads.erase(entry);

					continue;
				}
				// todo: what to do with SDL_CONTROLLERDEVICEREMAPPED?

				// mouse
				if (e.type == SDL_MOUSEMOTION)
				{
					auto const motion = glm::ivec2{ e.motion.xrel, -e.motion.yrel };

					if (motion.x != 0 || motion.y != 0)
						input_events.emplace(input::input_events::mouse_motion{ motion });

					continue;
				}
				if (e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEBUTTONDOWN)
				{
					auto const id = sdl_mousebutton_to_mouse_button(e.button.button);
					auto const value = sdl_buttonstate_to_button_state(e.button.state);

					input_events.emplace(input::input_events::mouse_button{ id, value });
					
					continue;
				}
				if (e.type == SDL_MOUSEWHEEL)
				{
					auto const motion = 
						(e.wheel.direction == SDL_MOUSEWHEEL_NORMAL) ? 
							glm::ivec2{  e.wheel.x,  e.wheel.y } :
							glm::ivec2{ -e.wheel.x, -e.wheel.y };
					
					if (motion.x != 0 || motion.y != 0)
						input_events.emplace(input::input_events::mouse_wheel{ motion });
					
					continue;
				}

				// keyboard
				if (e.type == SDL_KEYUP || e.type == SDL_KEYDOWN)
				{
					auto const id = sdl_scancode_to_keyboard_key(e.key.keysym.scancode);
					auto const value = sdl_buttonstate_to_button_state(e.key.state);

					input_events.emplace(input::input_events::keyboard_key{ id, value });
					
					continue;
				}

				// controller
				if (e.type == SDL_CONTROLLERAXISMOTION)
				{
					auto const id = sdl_controlleraxis_to_gamepad_axis(e.caxis.axis);
					auto const value = sdl_controller_axis_value_to_float(e.caxis.value);

					input_events.emplace(input::input_events::gamepad_axis{ id, value });
					
					continue;
				}
				if (e.type == SDL_CONTROLLERBUTTONUP || e.type == SDL_CONTROLLERBUTTONDOWN)
				{
					auto const id = sdl_controllerbutton_to_gamepad_button(e.cbutton.button);
					auto const value = sdl_buttonstate_to_button_state(e.cbutton.state);

					input_events.emplace(input::input_events::gamepad_button{ id, value });
					
					continue;
				}
			}
		}
		
	} // sdl
	
} // bump