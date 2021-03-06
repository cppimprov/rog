#pragma once

#include "bump_math.hpp"
#include "bump_ptr_handle.hpp"
#include "bump_sdl_blendmode.hpp"

#include <SDL.h>

namespace bump
{
	
	namespace sdl
	{
		
		class surface : public ptr_handle<SDL_Surface>
		{
		public:

			surface() = default;
			explicit surface(SDL_Surface* surface);
			explicit surface(glm::i32vec2 size);

			glm::i32vec2 get_size() const;

			void set_blend_mode(blend_mode mode);
		};

		void blit_surface(surface const& src, glm::i32vec2 src_pos, glm::i32vec2 src_size, surface const& dst, glm::i32vec2 dst_pos, glm::i32vec2 dst_size);
		
	} // sdl
	
} // bump
