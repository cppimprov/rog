#pragma once

#include "rog_thread_switch.hpp"

#include <bump_input.hpp>

#include <queue>

namespace rog
{
	
	struct thread_context
	{
		thread_switch m_switch;
		std::queue<bump::input::input_event> m_events;
		bool m_main_thread_request_quit = false;
		bool m_game_thread_done = false;
	};
	
} // rog
