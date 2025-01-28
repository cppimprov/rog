
#include "smirc.hpp"

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_log.hpp>
#include <bump_render_text.hpp>
#include <bump_transform.hpp>
#include <bump_ui.hpp>

namespace smirc
{

	struct ui_profile_dialog
	{
		std::shared_ptr<bump::ui::canvas> m_dialog;

		std::shared_ptr<bump::ui::vector_v> m_profiles_list;
		std::shared_ptr<bump::ui::label_button> m_new_profile_button; // todo: image and text?

		std::shared_ptr<bump::ui::canvas> m_form_panel;
		std::shared_ptr<bump::ui::text_field> m_field_nick;
		std::shared_ptr<bump::ui::text_field> m_field_user;
		std::shared_ptr<bump::ui::text_field> m_field_real;
	};

	ui_profile_dialog make_profile_dialog(bump::app& app)
	{
		namespace ui = bump::ui;

		auto const& fonts = app.m_assets.m_fonts;

		auto result = ui_profile_dialog();
		result.m_dialog = std::make_shared<ui::canvas>();
		result.m_dialog->origin = { ui::origin::center, ui::origin::center };

		auto dialog_vec = std::make_shared<ui::vector_v>();
		result.m_dialog->children.push_back(dialog_vec);

		// title bar
		{
			auto title_bar = std::make_shared<ui::label>(app.m_ft_context, fonts.at("title"), "Profiles");
			title_bar->margins = { 10, 0, 10, 0 };
			dialog_vec->children.push_back(title_bar);

			// todo: x button
		}

		// content
		{
			auto content_vec = std::make_shared<ui::vector_h>();
			content_vec->margins = { 10, 0, 10, 0 };
			content_vec->spacing = 20;
			dialog_vec->children.push_back(content_vec);

			// profiles list
			{
				result.m_profiles_list = std::make_shared<ui::vector_v>();
				result.m_profiles_list->fill = { ui::fill::fixed, ui::fill::shrink };
				result.m_profiles_list->size = { 100, 0 };
				content_vec->children.push_back(result.m_profiles_list);

				// todo: populate profiles list with stored profiles

				result.m_new_profile_button = std::make_shared<ui::label_button>(app.m_ft_context, fonts.at("title"), "new");
				result.m_new_profile_button->fill = { ui::fill::expand, ui::fill::shrink };
				result.m_profiles_list->children.push_back(result.m_new_profile_button);
			}

			// profile form
			{
				result.m_form_panel = std::make_shared<ui::canvas>();
				content_vec->children.push_back(result.m_form_panel);

				auto form_grid = std::make_shared<ui::grid>();
				form_grid->children.resize({ 2, 3 });
				result.m_form_panel->children.push_back(form_grid);

				auto nick_label = std::make_shared<ui::label>(app.m_ft_context, fonts.at("field"), "nick:");
				form_grid->children.at({ 0, 0 }) = nick_label;

				result.m_field_nick = std::make_shared<ui::text_field>(app.m_input_handler, app.m_ft_context, fonts.at("field"), "nick");
				result.m_field_nick->padding = { 10, 0, 10, 0 };
				form_grid->children.at({ 1, 0 }) = result.m_field_nick;

				auto user_label = std::make_shared<ui::label>(app.m_ft_context, fonts.at("field"), "user:");
				form_grid->children.at({ 0, 1 }) = user_label;

				result.m_field_user = std::make_shared<ui::text_field>(app.m_input_handler, app.m_ft_context, fonts.at("field"), "user");
				result.m_field_user->padding = { 10, 0, 10, 0 };
				form_grid->children.at({ 1, 1 }) = result.m_field_user;

				auto real_label = std::make_shared<ui::label>(app.m_ft_context, fonts.at("field"), "real:");
				form_grid->children.at({ 0, 2 }) = real_label;

				result.m_field_real = std::make_shared<ui::text_field>(app.m_input_handler, app.m_ft_context, fonts.at("field"), "real");
				result.m_field_real->padding = { 10, 0, 10, 0 };
				form_grid->children.at({ 1, 2 }) = result.m_field_real;
			}
		}

		return result;
	}

	bump::gamestate main_state(bump::app& app)
	{
		auto const& shaders = app.m_assets.m_shaders;

		auto ui_renderer = bump::ui::renderer(
			shaders.at("ui_rect"),
			shaders.at("ui_textured_rect"),
			shaders.at("ui_text"));

		namespace ui = bump::ui;

		auto ui_profile_dialog = make_profile_dialog(app);

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

					auto consumed = false;
					ui_profile_dialog.m_dialog->input(event, consumed);
				}
			}

			// update
			{
				// layout ui
				ui_profile_dialog.m_dialog->measure();
				ui_profile_dialog.m_dialog->place({ 0, 0 }, app.m_window.get_size());
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
				ui_profile_dialog.m_dialog->render(ui_renderer, renderer, camera);

				window.swap_buffers();
			}
		}

		return { };
	}

} // smirc
