#pragma once

#include "bump_math.hpp"

#include <functional>
#include <string>
#include <variant>

namespace bump
{
	
	namespace input
	{

		enum class keyboard_key
		{
			A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
			N1, N2, N3, N4, N5, N6, N7, N8, N9, N0,
			F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

			ESCAPE,

			PRINTSCREEN, SCROLLLOCK, PAUSE,

			BACKTICK, MINUS, EQUALS,
			LEFTSQUAREBRACKET, RIGHTSQUAREBRACKET,
			SEMICOLON, SINGLEQUOTE, HASH,
			BACKSLASH, COMMA, DOT, FORWARDSLASH,

			BACKSPACE, RETURN, TAB, CAPSLOCK,
			LEFTSHIFT, RIGHTSHIFT,
			LEFTCTRL, RIGHTCTRL,
			LEFTALT, RIGHTALT,
			SPACE,

			LEFTWINDOWS, RIGHTWINDOWS,
			CONTEXTMENU,

			INSERT, DELETE, HOME, END,
			PAGEUP, PAGEDOWN,

			NUMLOCK, NUMDIVIDE, NUMMULTIPLY, NUMMINUS, NUMPLUS, NUMENTER, NUMDOT,
			NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9, NUM0,

			ARROWLEFT, ARROWRIGHT, ARROWUP, ARROWDOWN,

			UNRECOGNISED,
		};
		
		std::string keyboard_key_to_string(keyboard_key key);
		keyboard_key keyboard_key_from_string(std::string const& str);

		enum class mouse_button
		{
			LEFT, MIDDLE, RIGHT,
			X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, X16, X17, X18, X19, X20,
		};
		
		std::string mouse_button_to_string(mouse_button key);
		mouse_button mouse_button_from_string(std::string const& str);

		enum class gamepad_button
		{
			X, Y, A, B,
			DPADLEFT, DPADRIGHT, DPADUP, DPADDOWN,
			LEFTBUMPER, RIGHTBUMPER,
			BACK, START, GUIDE,
			LEFTSTICK, RIGHTSTICK,
		};

		std::string gamepad_button_to_string(gamepad_button key);
		gamepad_button gamepad_button_from_string(std::string const& str);

		enum class gamepad_axis
		{
			STICK_LEFTX, STICK_LEFTY,
			STICK_RIGHTX, STICK_RIGHTY,

			TRIGGER_LEFT, TRIGGER_RIGHT,
		};
		
		std::string gamepad_axis_to_string(gamepad_axis key);
		gamepad_axis gamepad_axis_from_string(std::string const& str);

		enum class button_state { UP, DOWN };

		namespace input_events
		{
			struct keyboard_key   { input::keyboard_key m_key; button_state m_state; };
			struct mouse_button   { input::mouse_button m_button; button_state m_state; };
			struct mouse_wheel    { glm::ivec2 m_motion; };
			struct mouse_motion   { glm::ivec2 m_motion; };
			struct gamepad_button { input::gamepad_button m_button; button_state m_state; };
			struct gamepad_axis   { input::gamepad_axis m_axis; float m_value; };
		}

		using input_event = std::variant
		<
			input_events::keyboard_key,
			input_events::mouse_button,
			input_events::mouse_wheel,
			input_events::mouse_motion,
			input_events::gamepad_button,
			input_events::gamepad_axis
		>;

		namespace app_events
		{
			// app events
			struct resize { glm::ivec2 m_size; };
			struct pause  { bool m_pause; };
			struct quit   { };
		}

		using app_event = std::variant
		<
			app_events::resize,
			app_events::pause,
			app_events::quit
		>;

	} // input
	
} // bump
