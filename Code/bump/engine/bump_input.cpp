#include "bump_input.hpp"

#include "bump_pair_map.hpp"

namespace bump
{
	
	namespace input
	{
		
		namespace
		{

			auto const keyboard_key_strings = pair_map<keyboard_key, std::string>
			{
				{ keyboard_key::A, "A" },
				{ keyboard_key::B, "B" },
				{ keyboard_key::C, "C" },
				{ keyboard_key::D, "D" },
				{ keyboard_key::E, "E" },
				{ keyboard_key::F, "F" },
				{ keyboard_key::G, "G" },
				{ keyboard_key::H, "H" },
				{ keyboard_key::I, "I" },
				{ keyboard_key::J, "J" },
				{ keyboard_key::K, "K" },
				{ keyboard_key::L, "L" },
				{ keyboard_key::M, "M" },
				{ keyboard_key::N, "N" },
				{ keyboard_key::O, "O" },
				{ keyboard_key::P, "P" },
				{ keyboard_key::Q, "Q" },
				{ keyboard_key::R, "R" },
				{ keyboard_key::S, "S" },
				{ keyboard_key::T, "T" },
				{ keyboard_key::U, "U" },
				{ keyboard_key::V, "V" },
				{ keyboard_key::W, "W" },
				{ keyboard_key::X, "X" },
				{ keyboard_key::Y, "Y" },
				{ keyboard_key::Z, "Z" },

				{ keyboard_key::N1, "1" },
				{ keyboard_key::N2, "2" },
				{ keyboard_key::N3, "3" },
				{ keyboard_key::N4, "4" },
				{ keyboard_key::N5, "5" },
				{ keyboard_key::N6, "6" },
				{ keyboard_key::N7, "7" },
				{ keyboard_key::N8, "8" },
				{ keyboard_key::N9, "9" },
				{ keyboard_key::N0, "0" },

				{ keyboard_key::F1, "F1" },
				{ keyboard_key::F2, "F2" },
				{ keyboard_key::F3, "F3" },
				{ keyboard_key::F4, "F4" },
				{ keyboard_key::F5, "F5" },
				{ keyboard_key::F6, "F6" },
				{ keyboard_key::F7, "F7" },
				{ keyboard_key::F8, "F8" },
				{ keyboard_key::F9, "F9" },
				{ keyboard_key::F10, "F10" },
				{ keyboard_key::F11, "F11" },
				{ keyboard_key::F12, "F12" },

				{ keyboard_key::ESCAPE, "ESCAPE" },

				{ keyboard_key::PRINTSCREEN, "PRINTSCREEN" },
				{ keyboard_key::SCROLLLOCK, "SCROLLLOCK" },
				{ keyboard_key::PAUSE, "PAUSE" },

				{ keyboard_key::BACKTICK, "BACKTICK" },
				{ keyboard_key::MINUS, "MINUS" },
				{ keyboard_key::EQUALS, "EQUALS" },

				{ keyboard_key::LEFTSQUAREBRACKET, "LEFTSQUAREBRACKET" },
				{ keyboard_key::RIGHTSQUAREBRACKET, "RIGHTSQUAREBRACKET" },

				{ keyboard_key::SEMICOLON, "SEMICOLON" },
				{ keyboard_key::SINGLEQUOTE, "SINGLEQUOTE" },
				{ keyboard_key::HASH, "HASH" },

				{ keyboard_key::BACKSLASH, "BACKSLASH" },
				{ keyboard_key::COMMA, "COMMA" },
				{ keyboard_key::DOT, "DOT" },
				{ keyboard_key::FORWARDSLASH, "FORWARDSLASH" },

				{ keyboard_key::BACKSPACE, "BACKSPACE" },
				{ keyboard_key::RETURN, "RETURN" },
				{ keyboard_key::TAB, "TAB" },
				{ keyboard_key::CAPSLOCK, "CAPSLOCK" },

				{ keyboard_key::LEFTSHIFT, "LEFTSHIFT" },
				{ keyboard_key::RIGHTSHIFT, "RIGHTSHIFT" },

				{ keyboard_key::LEFTCTRL, "LEFTCTRL" },
				{ keyboard_key::RIGHTCTRL, "RIGHTCTRL" },

				{ keyboard_key::LEFTALT, "LEFTALT" },
				{ keyboard_key::RIGHTALT, "RIGHTALT" },

				{ keyboard_key::SPACE, "SPACE" },

				{ keyboard_key::LEFTWINDOWS, "LEFTWINDOWS" },
				{ keyboard_key::RIGHTWINDOWS, "RIGHTWINDOWS" },

				{ keyboard_key::CONTEXTMENU, "CONTEXTMENU" },

				{ keyboard_key::INSERT, "INSERT" },
				{ keyboard_key::DELETE, "DELETE" },
				{ keyboard_key::HOME, "HOME" },
				{ keyboard_key::END, "END" },

				{ keyboard_key::PAGEUP, "PAGEUP" },
				{ keyboard_key::PAGEDOWN, "PAGEDOWN" },

				{ keyboard_key::NUMLOCK, "NUMLOCK" },
				{ keyboard_key::NUMDIVIDE, "NUMDIVIDE" },
				{ keyboard_key::NUMMULTIPLY, "NUMMULTIPLY" },
				{ keyboard_key::NUMMINUS, "NUMMINUS" },
				{ keyboard_key::NUMPLUS, "NUMPLUS" },
				{ keyboard_key::NUMENTER, "NUMENTER" },
				{ keyboard_key::NUMDOT, "NUMDOT" },

				{ keyboard_key::NUM1, "NUM1" },
				{ keyboard_key::NUM2, "NUM2" },
				{ keyboard_key::NUM3, "NUM3" },
				{ keyboard_key::NUM4, "NUM4" },
				{ keyboard_key::NUM5, "NUM5" },
				{ keyboard_key::NUM6, "NUM6" },
				{ keyboard_key::NUM7, "NUM7" },
				{ keyboard_key::NUM8, "NUM8" },
				{ keyboard_key::NUM9, "NUM9" },
				{ keyboard_key::NUM0, "NUM0" },

				{ keyboard_key::ARROWLEFT, "ARROWLEFT" },
				{ keyboard_key::ARROWRIGHT, "ARROWRIGHT" },
				{ keyboard_key::ARROWUP, "ARROWUP" },
				{ keyboard_key::ARROWDOWN, "ARROWDOWN" },

				{ keyboard_key::UNRECOGNISED, "UNRECOGNISED" },
			};


			auto const mouse_button_strings = pair_map<mouse_button, std::string>
			{
				{ mouse_button::LEFT, "LEFT" },
				{ mouse_button::MIDDLE, "MIDDLE" },
				{ mouse_button::RIGHT, "RIGHT" },

				{ mouse_button::X1, "X1" },
				{ mouse_button::X2, "X2" },
				{ mouse_button::X3, "X3" },
				{ mouse_button::X4, "X4" },
				{ mouse_button::X5, "X5" },
				{ mouse_button::X6, "X6" },
				{ mouse_button::X7, "X7" },
				{ mouse_button::X8, "X8" },
				{ mouse_button::X9, "X9" },
				{ mouse_button::X10, "X10" },
				{ mouse_button::X11, "X11" },
				{ mouse_button::X12, "X12" },
				{ mouse_button::X13, "X13" },
				{ mouse_button::X14, "X14" },
				{ mouse_button::X15, "X15" },
				{ mouse_button::X16, "X16" },
				{ mouse_button::X17, "X17" },
				{ mouse_button::X18, "X18" },
				{ mouse_button::X19, "X19" },
				{ mouse_button::X20, "X20" },
			};

			auto const gamepad_button_strings = pair_map<gamepad_button, std::string>
			{
				{ gamepad_button::X, "X" },
				{ gamepad_button::Y, "Y" },
				{ gamepad_button::A, "A" },
				{ gamepad_button::B, "B" },

				{ gamepad_button::DPADLEFT, "DPADLEFT" },
				{ gamepad_button::DPADRIGHT, "DPADRIGHT" },
				{ gamepad_button::DPADUP, "DPADUP" },
				{ gamepad_button::DPADDOWN, "DPADDOWN" },

				{ gamepad_button::LEFTBUMPER, "LEFTBUMPER" },
				{ gamepad_button::RIGHTBUMPER, "RIGHTBUMPER" },

				{ gamepad_button::LEFTSTICK, "LEFTSTICK" },
				{ gamepad_button::RIGHTSTICK, "RIGHTSTICK" },

				{ gamepad_button::BACK, "BACK" },
				{ gamepad_button::START, "START" },
				{ gamepad_button::GUIDE, "GUIDE" },
			};

			auto const gamepad_axis_strings = pair_map<gamepad_axis, std::string>
			{
				{ gamepad_axis::STICK_LEFTX, "STICK_LEFTX" },
				{ gamepad_axis::STICK_LEFTY, "STICK_LEFTY" },
				{ gamepad_axis::STICK_RIGHTX, "STICK_RIGHTX" },
				{ gamepad_axis::STICK_RIGHTY, "STICK_RIGHTY" },

				{ gamepad_axis::TRIGGER_LEFT, "TRIGGER_LEFT" },
				{ gamepad_axis::TRIGGER_RIGHT, "TRIGGER_RIGHT" },

			};

		} // unnamed
	
		std::string keyboard_key_to_string(keyboard_key key)
		{
			return keyboard_key_strings.get_second(key);
		}

		keyboard_key keyboard_key_from_string(std::string const& str)
		{
			return keyboard_key_strings.get_first(str);
		}
		
		std::string mouse_button_to_string(mouse_button key)
		{
			return mouse_button_strings.get_second(key);
		}

		mouse_button mouse_button_from_string(std::string const& str)
		{
			return mouse_button_strings.get_first(str);
		}
		
		std::string gamepad_button_to_string(gamepad_button key)
		{
			return gamepad_button_strings.get_second(key);
		}

		gamepad_button gamepad_button_from_string(std::string const& str)
		{
			return gamepad_button_strings.get_first(str);
		}

		std::string gamepad_axis_to_string(gamepad_axis key)
		{
			return gamepad_axis_strings.get_second(key);
		}

		gamepad_axis gamepad_axis_from_string(std::string const& str)
		{
			return gamepad_axis_strings.get_first(str);
		}
		
	} // input
	
} // bump