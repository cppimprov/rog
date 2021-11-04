
#include <bump_app.hpp>
#include <bump_assets.hpp>
#include <bump_gamestate.hpp>
#include <bump_log.hpp>

#include <SDL.h>
#include <SDL_main.h>

#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>

#include <bump_camera.hpp>
#include <bump_gl.hpp>
#include <bump_grid.hpp>
#include <bump_input.hpp>
#include <bump_timer.hpp>
#include <bump_transform.hpp>
#include <bump_range.hpp>
#include <bump_render_text.hpp>

#include "rog_screen_buffer.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/std_based_type.hpp>

namespace rog
{

	class tile_renderable
	{
	public:

		explicit tile_renderable(bump::gl::shader_program const& shader);

		tile_renderable(tile_renderable const&) = delete;
		tile_renderable& operator=(tile_renderable const&) = delete;

		tile_renderable(tile_renderable &&) = default;
		tile_renderable& operator=(tile_renderable &&) = default;

		void render(bump::gl::renderer& renderer, bump::camera_matrices const& matrices, std::vector<glm::vec2> const& positions, glm::vec2 tile_size);

	private:

		bump::gl::shader_program const* m_shader;
		GLint m_in_VertexPosition;
		GLint m_in_TilePosition;
		GLint m_u_TileSize;
		GLint m_u_MVP;

		bump::gl::buffer m_vertex_buffer;
		bump::gl::buffer m_tile_position_buffer;
		bump::gl::vertex_array m_vertex_array;
	};

	tile_renderable::tile_renderable(bump::gl::shader_program const& shader):
		m_shader(&shader),
		m_in_VertexPosition(shader.get_attribute_location("in_VertexPosition")),
		m_in_TilePosition(shader.get_attribute_location("in_TilePosition")),
		m_u_TileSize(shader.get_uniform_location("u_TileSize")),
		m_u_MVP(shader.get_uniform_location("u_MVP"))
	{
		auto vertices = { 0.f, 0.f,  1.f, 0.f,  1.f, 1.f,  0.f, 0.f,  1.f, 1.f,  0.f, 1.f, };
		m_vertex_buffer.set_data(GL_ARRAY_BUFFER, vertices.begin(), 2, 6, GL_STATIC_DRAW);
		m_vertex_array.set_array_buffer(m_in_VertexPosition, m_vertex_buffer);

		m_tile_position_buffer.set_data(GL_ARRAY_BUFFER, (float*)nullptr, 2, 0, GL_STREAM_DRAW);
		m_vertex_array.set_array_buffer(m_in_TilePosition, m_tile_position_buffer, 1);
	}
	
	void tile_renderable::render(bump::gl::renderer& renderer, bump::camera_matrices const& matrices, std::vector<glm::vec2> const& positions, glm::vec2 tile_size)
	{
		auto const instance_count = positions.size();

		if (instance_count == 0)
			return;

		m_tile_position_buffer.set_data(GL_ARRAY_BUFFER, glm::value_ptr(positions.front()), 2, instance_count, GL_STREAM_DRAW);

		renderer.set_program(*m_shader);
		renderer.set_uniform_2f(m_u_TileSize, tile_size);
		renderer.set_uniform_4x4f(m_u_MVP, matrices.model_view_matrix(glm::identity<glm::mat4>()));
		renderer.set_vertex_array(m_vertex_array);

		renderer.draw_arrays(GL_TRIANGLES, m_vertex_buffer.get_element_count(), instance_count);

		renderer.clear_vertex_array();
		renderer.clear_program();
	}

	bump::gamestate do_start(bump::app& app)
	{
		using namespace bump;

		log_info("start state");

		auto const tile_size_i = glm::i32vec2(64);
		auto const tile_size_f = glm::vec2(tile_size_i);
		auto const ascii_tiles = render_ascii_tiles(app.m_ft_context, app.m_assets.m_fonts.at("tiles"), tile_size_i);

		auto tile = tile_renderable(app.m_assets.m_shaders.at("tile"));

		auto screen = screen_buffer({ 80, 24 }, '#');
		screen.fill_rect({ 1, 1 }, screen.get_size() - glm::size2(2), '.');

		auto paused = false;
		auto timer = frame_timer();

		while (true)
		{
			// input
			{
				auto quit = false;
				auto callbacks = input::input_callbacks();
				callbacks.m_quit = [&] () { quit = true; };
				callbacks.m_pause = [&] (bool pause) { paused = pause; if (!paused) timer = frame_timer(); };

				app.m_input_handler.poll_input(callbacks);

				if (quit)
					return { };
			}

			// update
			{
				// auto const dt = paused ? frame_timer::clock_t::duration{ 0 } : timer.get_last_frame_time();

				// ...
			}

			// render
			{
				auto const& window_size = app.m_window.get_size();
				auto const window_size_f = glm::vec2(window_size);
				auto const window_size_u = glm::uvec2(window_size);

				auto camera = bump::orthographic_camera();
				// camera.m_projection.m_near = 0.f;
				// camera.m_projection.m_near = 10.f;
				camera.m_projection.m_size = window_size;
				camera.m_viewport.m_size = window_size;
				//rotate_around_local_axis(camera.m_transform, glm::vec3{ 1.f, 0.f, 0.f }, glm::radians(-90.f));
				//translate_in_local(camera.m_transform, glm::vec3{ -1.f, 0.f, 0.f });

				// auto camera = bump::perspective_camera();
				// camera.m_projection.m_size = window_size;
				// camera.m_viewport.m_size = window_size;
				// translate_in_local(camera.m_transform, glm::vec3{ 0.f, 0.f, 5.f });

				auto& renderer = app.m_renderer;

				renderer.clear_color_buffers({ 0.f, 0.f, 0.f, 1.f });
				renderer.clear_depth_buffers();
				renderer.set_viewport({ 0, 0 }, window_size_u);

				auto const matrices = camera_matrices(camera);
				tile.render(renderer, matrices, { { 0.f, 0.f } }, tile_size_f);

				app.m_window.swap_buffers();
			}

			timer.tick();
		}
		
		return { };
	}

} // rog

int main(int , char* [])
{
	
	{
		
		auto const metadata = bump::asset_metadata
		{
			// fonts
			{
				{ "tiles", "RobotoMono-SemiBold.ttf", 30 },
			},
			// sounds
			{
				// { "intro", "intro.wav" },
			},
			// music
			{
				// { "intro", "intro.wav" },
			},
			// shaders
			{
				{ "tile", { "tile.vert", "tile.frag" } },
			},
			// models
			{
				// { "skybox", "skybox.mbp_model" },
			},
			// cubemaps
			{
				// { "skybox", { "skybox_x_pos.png", "skybox_x_neg.png", "skybox_y_pos.png", "skybox_y_neg.png", "skybox_z_pos.png", "skybox_z_neg.png" } },
			},
		};

		auto app = bump::app(metadata, { 1024, 768 }, "rog", bump::sdl::window::display_mode::WINDOWED);
		bump::run_state({ rog::do_start }, app);
	}

	bump::log_info("done!");

	return EXIT_SUCCESS;
}

// todo: 

	// basic font rendering:

		// rendering
			// tile_renderable class (instanced quad rendering)
			// shader

	// build script:
		// rc stage to add icon (yellow @ sign)
