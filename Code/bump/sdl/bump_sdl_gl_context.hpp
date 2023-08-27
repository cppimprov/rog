#pragma once

#include "bump_ptr_handle.hpp"

namespace bump
{
	
	namespace sdl
	{

		class window;
		
		class gl_context : public ptr_handle<void>
		{
		public:

			enum class swap_interval_mode { IMMEDIATE, VSYNC, ADAPTIVE_VSYNC };

			explicit gl_context(window const& window);

			void set_swap_interval(swap_interval_mode interval);
			swap_interval_mode get_swap_interval() const;
		};
		
	} // sdl
	
} // bump