
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
		
		// auto tab_bar = bump::ui::make_widget<bump::ui::h_vector>();
		// tab_bar->spacing = { 10, 0 };

		// 	auto add_button = bump::ui::make_widget<bump::ui::textured_button>();
		// 	add_button->set_texture(app.m_assets.m_textures_2d.at("add_button"));
		// 	tab_bar->children.push_back(add_button);

		// 	auto server_tab_group = bump::ui::make_widget<bump::ui::v_vector>();
		// 	server_tab_group->spacing = { 0, 10 };
		// 	tab_bar->children.push_back(server_tab_group);

		// 		auto server_label = bump::ui::make_widget<bump::ui::textured_button>();
		// 		server_label->set_texture(/* render text */);
		// 		server_tab_group->children.push_back(server_label);

		// 		auto channel_tab_group = bump::ui::make_widget<bump::ui::h_vector>();
		// 		channel_tab_group->spacing = { 10, 0 };
		// 		server_tab_group->children.push_back(channel_tab_group);

		// 			auto channel_tab_1 = bump::ui::make_widget<bump::ui::textured_button>();
		// 			channel_tab_1->set_texture(/* render text */);
		// 			channel_tab_group->children.push_back(channel_tab_1);

		// 			auto channel_tab_2 = bump::ui::make_widget<bump::ui::textured_button>();
		// 			channel_tab_2->set_texture(/* render text */);
		// 			channel_tab_group->children.push_back(channel_tab_2);

		namespace ui = bump::ui;

		auto ui_test_quad1 = std::make_shared<ui::quad>(app.m_assets.m_shaders.at("ui_quad"));
		ui_test_quad1->size = { 50, 50 };
		ui_test_quad1->color = { 1.f, 0.8f, 0.2f, 1.f };

		auto ui_test_quad2 = std::make_shared<ui::quad>(app.m_assets.m_shaders.at("ui_quad"));
		ui_test_quad2->size = { 100, 50 };
		ui_test_quad2->color = { 0.6f, 0.6f, 1.f, 1.f };

		auto ui_test_quad3 = std::make_shared<ui::textured_quad>(app.m_assets.m_shaders.at("ui_textured_quad"), app.m_assets.m_textures_2d.at("plus"));

		auto ui_test_quad4 = std::make_shared<ui::label>(app.m_assets.m_shaders.at("ui_label"), app.m_ft_context, app.m_assets.m_fonts.at("menu"), "test label");
		ui_test_quad4->margins = { 20, 20, 20, 20 };
	
		auto ui_test_grid = std::make_shared<ui::grid>();
		ui_test_grid->children.resize({ 2, 2 });
		ui_test_grid->children.at({ 0, 0 }) = ui_test_quad1;
		ui_test_grid->children.at({ 1, 0 }) = ui_test_quad2;
		ui_test_grid->children.at({ 0, 1 }) = ui_test_quad3;
		ui_test_grid->children.at({ 1, 1 }) = ui_test_quad4;

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

					// todo: typing
					// todo: mouse input
				}
			}

			// update
			{
				// layout ui
				ui_test_grid->measure();
				ui_test_grid->place({ 0, 0 }, app.m_window.get_size());
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
				ui_test_grid->render(renderer, camera);

				window.swap_buffers();
			}
		}

		return { };
	}

} // smirc


// ui:
	// buttons
	// text boxes / text fields
	// labels
	// ...
