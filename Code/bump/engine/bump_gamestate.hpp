#pragma once

#include <functional>

namespace bump
{
	
	class app;
	
	class gamestate
	{
	public:
		
		std::function<gamestate(app&)> m_function;
	};
	
	void run_state(gamestate state, app& app);
	
} // bump
