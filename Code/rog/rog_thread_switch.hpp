#pragma once

#include <condition_variable>
#include <mutex>

namespace rog
{
	
	class thread_switch
	{
	public:
		
		thread_switch() = default;

		void notify_main_thread_and_wait();
		void notify_game_thread_and_wait();
		
		void notify_main_thread();

	private:
		
		std::mutex m_mutex;
		
		std::condition_variable m_cv_main;
		bool m_ready_main;
		
		std::condition_variable m_cv_game;
		bool m_ready_game;
	};
	
} // rog
