
#include "smirc.hpp"

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_log.hpp>
#include <bump_render_text.hpp>
#include <bump_transform.hpp>
#include <bump_ui.hpp>

namespace smirc
{

	bump::gamestate main_state(bump::app& app)
	{
		
		namespace ui = bump::ui;

		auto tab_bar = std::make_shared<ui::canvas>();
		tab_bar->fill = { ui::fill::expand, ui::fill::shrink };

		auto l_vec = std::make_shared<ui::vector_h>();
		l_vec->spacing = 20;
		l_vec->origin = { ui::origin::left, ui::origin::top };
		tab_bar->children.push_back(l_vec);
		
		auto plus = std::make_shared<ui::textured_quad>(app.m_assets.m_shaders.at("ui_textured_quad"), app.m_assets.m_textures_2d.at("plus"));
		l_vec->children.push_back(plus);
		
		auto search = std::make_shared<ui::quad>(app.m_assets.m_shaders.at("ui_quad"));
		search->origin = { ui::origin::right, ui::origin::top };
		search->size = { 100, 100 };
		search->color = { 1.0f, 0.8f, 0.2f, 1.0f };
		tab_bar->children.push_back(search);

		auto server_tab_vec = std::make_shared<ui::vector_h>();
		server_tab_vec->spacing = 20;
		l_vec->children.push_back(server_tab_vec);

		// todo: what to do about scrolling server tabs?

		{
			auto server_tab_1 = std::make_shared<ui::vector_v>();
			server_tab_vec->children.push_back(server_tab_1);
			
			auto server_tab_1_label = std::make_shared<ui::label>(app.m_assets.m_shaders.at("ui_label"), app.m_ft_context, app.m_assets.m_fonts.at("menu"), "server_name");
			server_tab_1->children.push_back(server_tab_1_label);

			auto server_tab_1_channels = std::make_shared<ui::vector_h>();
			server_tab_1->children.push_back(server_tab_1_channels);
			
			auto channel_1_label = std::make_shared<ui::label>(app.m_assets.m_shaders.at("ui_label"), app.m_ft_context, app.m_assets.m_fonts.at("menu"), "channel_1");
			server_tab_1_channels->children.push_back(channel_1_label);

			auto channel_2_label = std::make_shared<ui::label>(app.m_assets.m_shaders.at("ui_label"), app.m_ft_context, app.m_assets.m_fonts.at("menu"), "channel_2");
			server_tab_1_channels->children.push_back(channel_2_label);
		}
		
		{
			auto server_tab_1 = std::make_shared<ui::vector_v>();
			server_tab_vec->children.push_back(server_tab_1);
			
			auto server_tab_1_label = std::make_shared<ui::label_button>(app.m_assets.m_shaders.at("ui_label"), app.m_ft_context, app.m_assets.m_fonts.at("menu"), "server_name");
			server_tab_1->children.push_back(server_tab_1_label);

			auto server_tab_1_channels = std::make_shared<ui::vector_h>();
			server_tab_1->children.push_back(server_tab_1_channels);
			
			auto channel_1_label = std::make_shared<ui::label_button>(app.m_assets.m_shaders.at("ui_label"), app.m_ft_context, app.m_assets.m_fonts.at("menu"), "channel_1");
			server_tab_1_channels->children.push_back(channel_1_label);

			auto channel_2_label = std::make_shared<ui::label_button>(app.m_assets.m_shaders.at("ui_label"), app.m_ft_context, app.m_assets.m_fonts.at("menu"), "channel_2");
			server_tab_1_channels->children.push_back(channel_2_label);
		}

		auto canvas = std::make_shared<ui::canvas>();
		canvas->fill = { ui::fill::expand, ui::fill::expand };

		auto dialog = std::make_shared<ui::vector_v>();
		dialog->spacing = 20;
		dialog->origin = { ui::origin::center, ui::origin::center };
		canvas->children.push_back(dialog);

		auto text_field_1 = std::make_shared<ui::text_field>(app.m_assets.m_shaders.at("ui_label"), app.m_ft_context, app.m_assets.m_fonts.at("menu"), "test 1");
		dialog->children.push_back(text_field_1);

		auto text_field_2 = std::make_shared<ui::text_field>(app.m_assets.m_shaders.at("ui_label"), app.m_ft_context, app.m_assets.m_fonts.at("menu"), "test 2");
		dialog->children.push_back(text_field_2);

		auto app_events = std::queue<bump::input::app_event>();
		auto input_events = std::queue<bump::input::input_event>();

		while (true)
		{
			// input
			{
				app.m_input_handler.poll(app_events, input_events);

				// process app events
				while (!app_events.empty())
				{
					auto event = std::move(app_events.front());
					app_events.pop();

					namespace ae = bump::input::app_events;

					if (std::holds_alternative<ae::quit>(event))
						return { }; // todo: save!

					// todo: pause
					// todo: resize
				}

				// process input events
				while (!input_events.empty())
				{
					auto event = std::move(input_events.front());
					input_events.pop();

					namespace ie = bump::input::input_events;

					if (std::holds_alternative<ie::keyboard_key>(event))
					{
						auto const& k = std::get<ie::keyboard_key>(event);

						using kt = bump::input::keyboard_key;

						// temp:
						if (k.m_key == kt::ESCAPE && k.m_value)
							return { }; // todo: save!
					}

					tab_bar->input(event);

					canvas->input(event);

					// todo: typing
					// todo: mouse input
				}
			}

			// update
			{
				// layout ui
				tab_bar->measure();
				tab_bar->place({ 0, 0 }, app.m_window.get_size());

				canvas->measure();
				canvas->place({ 0, 0 }, app.m_window.get_size());
			}

			// drawing
			{
				auto& window = app.m_window;
				auto& renderer = app.m_renderer;

				auto const window_size_u = glm::uvec2(window.get_size());
				auto const window_size_f = glm::vec2(window.get_size());

				// setup
				renderer.clear_color_buffers({ 0.f, 0.f, 0.f, 1.f });
				renderer.clear_depth_buffers();
				renderer.set_viewport({ 0, 0 }, window_size_u);
				renderer.set_blending(bump::gl::renderer::blending::BLEND);

				// setup camera
				// note: y direction is flipped (so the origin is at the top left
				// and the y coordinates increase downwards)
				auto camera = bump::orthographic_camera();
				camera.m_projection.m_position.x = 0.f;
				camera.m_projection.m_position.y = window_size_f.y;
				camera.m_projection.m_size.x = window_size_f.x;
				camera.m_projection.m_size.y = -window_size_f.y;
				camera.m_viewport.m_size = window_size_f;

				// render
				tab_bar->render(renderer, camera);
				canvas->render(renderer, camera);

				window.swap_buffers();
			}
		}

		return { };
	}

} // smirc


// ui:
	// basic text field!
	// new connection screen
