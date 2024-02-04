#include "bump_sdl_context.hpp"

namespace bump
{
	
	namespace sdl
	{

		void context::quit()
		{
			if (m_active)
			{
				sdl::quit();
				m_active = false;
			}
		}

		result<context, std::string_view> init()
		{
			return init(subsystems::EVERYTHING);
		}

		result<context, std::string_view> init(subsystems s)
		{
			if (SDL_Init(s) != 0)
				return make_err(std::string_view(SDL_GetError()));

			return make_ok(context(true));
		}

		result<void, std::string_view> init_subsystems(subsystems s)
		{
			if (SDL_InitSubSystem(s) != 0)
				return make_err(std::string_view(SDL_GetError()));

			return make_ok();
		}

		subsystems was_init()
		{
			return was_init(subsystems::EVERYTHING);
		}

		subsystems was_init(subsystems s)
		{
			return static_cast<subsystems>(SDL_WasInit(s));
		}

		void quit()
		{
			SDL_Quit();
		}

		void quit_subsystems(subsystems s)
		{
			SDL_QuitSubSystem(s);
		}

	} // sdl
	
} // bump