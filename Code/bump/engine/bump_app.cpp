#include "bump_app.hpp"

namespace bump
{

	app::app(asset_metadata const& metadata, glm::i32vec2 window_size, std::string const& window_title, sdl::window::display_mode window_display_mode):
		m_sdl_context(),
		m_mixer_context(),
		m_ft_context(),
		m_window(window_size, window_title, window_display_mode),
		m_input_handler(m_window),
		m_gl_context(m_window),
		m_glew_context(),
		m_renderer(),
		m_enet_context(enet::initialize()),
		m_assets(load_assets(*this, metadata))
	{
		m_window.set_min_size({ 640, 360 });
	}

} // bump
