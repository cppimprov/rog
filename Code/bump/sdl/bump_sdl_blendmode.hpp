#pragma once

#include <SDL.h>

namespace bump
{
	
	namespace sdl
	{
		
		enum class blend_mode { NONE, BLEND, ADDITIVE, MODULATE };
	
		SDL_BlendMode get_sdl_blendmode(blend_mode mode);
		
	} // sdl
	
} // bump