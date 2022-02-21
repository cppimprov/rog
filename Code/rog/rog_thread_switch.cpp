#include "rog_thread_switch.hpp"

namespace rog
{
	
	void thread_switch::notify_main_thread_and_wait()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_ready_main = true;
		m_ready_game = false;
		m_cv_main.notify_one();
		m_cv_game.wait(lock, [&] () { return m_ready_game; });
	}
	
	void thread_switch::notify_game_thread_and_wait()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_ready_main = false;
		m_ready_game = true;
		m_cv_game.notify_one();
		m_cv_main.wait(lock, [&] () { return m_ready_main; });
	}
	
	void thread_switch::notify_main_thread()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_ready_main = true;
		m_cv_main.notify_one();
	}

} // rog
