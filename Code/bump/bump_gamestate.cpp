#include "bump_gamestate.hpp"

namespace bump
{
	
	void run_state(gamestate state, app& app)
	{
		while (state.m_function)
			state = state.m_function(app);
	}
		
} // bump
