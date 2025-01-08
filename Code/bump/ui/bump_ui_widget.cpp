#include "bump_ui_widget.hpp"

#include "bump_narrow_cast.hpp"
#include "bump_render_text.hpp"

#include <SDL.h>

#include <iostream>

namespace bump::ui
{

	void quad::render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera)
	{
		ui_renderer.draw_rect(gl_renderer, camera, position, size, color);
	}
	
	void textured_quad::render(ui::renderer const& ui_renderer, bump::gl::renderer& gl_renderer, camera_matrices const& camera)
	{
		ui_renderer.draw_textured_rect(gl_renderer, camera, position, size, *m_texture);
	}
	
	label::label(font::ft_context const& ft_context, font::font_asset const& font, std::string const& text):
		m_text(ft_context, font.m_ft_font, font.m_hb_font, text)
	{
		redraw_text();
	}

	void label::set_text(std::string const& text)
	{
		m_text.set(text);
		redraw_text();
	}

	void label::measure()
	{
		auto const width = m_texture.m_pos.x + m_texture.m_advance.x + padding.x + padding.z;
		auto const height = m_text.get_ft_font().get_line_height_px() + padding.y + padding.w;
		size = { width, height };
	}

	void label::render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera)
	{
		ui_renderer.draw_rect(gl_renderer, camera, position, size, bg_color);
		ui_renderer.draw_text(gl_renderer, camera, position + vec{ padding.x, padding.y }, m_texture, m_text.get_ft_font().get_line_height_px(), color);
	}

	void label::redraw_text()
	{
		m_texture = m_text.render();
	}

	label_button::label_button(font::ft_context const& ft_context, font::font_asset const& font, std::string const& text):
		m_text(ft_context, font.m_ft_font, font.m_hb_font, text),
		m_hovered(false),
		m_pressed(false)
	{
		redraw_text();
	}

	void label_button::set_text(std::string const& text)
	{
		m_text.set(text);
		redraw_text();
	}

	void label_button::measure()
	{
		auto const width = m_texture.m_pos.x + m_texture.m_advance.x + padding.x + padding.z;
		auto const height = m_text.get_ft_font().get_line_height_px() + padding.y + padding.w;
		size = { width, height };
	}

	void label_button::input(input::input_event const& event)
	{
		namespace ie = input::input_events;

		if (std::holds_alternative<ie::mouse_motion>(event))
		{
			auto const& mm = std::get<ie::mouse_motion>(event);
			m_hovered = aabb{ position, size }.contains(mm.m_inv_y_position);
			m_pressed = m_pressed && m_hovered;
		}

		if (std::holds_alternative<ie::mouse_button>(event))
		{
			auto const& mb = std::get<ie::mouse_button>(event);
			
			if (mb.m_button == input::mouse_button::LEFT)
			{
				if (mb.m_value)
				{
					if (m_hovered)
					{
						m_pressed = true;
						if (action) action();
					}
					else
					{
						m_pressed = false;
					}
				}
				else
				{
					m_pressed = false;
				}
			}
		}
	}

	void label_button::render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera)
	{
		auto const color = m_pressed ? press_color : m_hovered ? hover_color : inactive_color;

		ui_renderer.draw_rect(gl_renderer, camera, position, size, bg_color);
		ui_renderer.draw_text(gl_renderer, camera, position + vec{ padding.x, padding.y }, m_texture, m_text.get_ft_font().get_line_height_px(), color);
	}

	void label_button::redraw_text()
	{
		m_texture = m_text.render();
	}
	
	text_field::text_field(font::ft_context const& ft_context, font::font_asset const& font, std::string const& text):
		m_text(ft_context, font.m_ft_font, font.m_hb_font, text),
		m_hovered(false),
		m_pressed(false),
		m_focused(false),
		m_min_width_px(100),
		m_caret(0),
		m_selection(0),
		m_composition(0),
		m_caret_pos_px(0),
		m_selection_pos_px(0),
		m_composition_pos_px(0)
	{
		set_caret(m_text.size(), false, false);
		redraw_text();
	}

	void text_field::set_text(std::string const& text, bool select)
	{
		m_text.set(text);

		set_caret(0, false, false);
		set_caret(m_text.size(), select, false);

		redraw_text();
	}

	void text_field::set_max_length(std::size_t length)
	{
		m_text.set_max_length(length);

		m_caret = std::min(m_caret, m_text.size());
		m_selection = std::min(m_selection, m_text.size());

		m_caret_pos_px = m_text.measure(0, m_caret);
		m_selection_pos_px = m_text.measure(0, m_selection);
		m_composition_pos_px = m_text.measure(0, m_composition);
		
		redraw_text();
	}

	void text_field::set_caret(std::size_t pos, bool select, bool compose)
	{
		m_caret = std::min(pos, m_text.size());

		if (!select)
			m_selection = m_caret;

		if (!compose)
			m_composition = m_caret;

		m_caret_pos_px = m_text.measure(0, m_caret);
		m_selection_pos_px = m_text.measure(0, m_selection);
		m_composition_pos_px = m_text.measure(0, m_composition);
	}

	void text_field::move_caret(std::ptrdiff_t diff, cursor_mode mode, bool select)
	{
		// we have a selection, but we want to clear it and move the caret to the appropriate end
		if (selection_size() != 0 && !select)
		{
			set_caret(diff < 0 ? selection_start() : selection_end(), false, false);
			return;
		}

		auto const caret_pos = 
			mode == cursor_mode::WORD ?
				m_text.next_word(m_caret, diff) : 
				mode == cursor_mode::CLUSTER ?
					m_text.next_cluster(m_caret, diff) :
					m_text.next_codepoint(m_caret, diff);
		
		set_caret(caret_pos, select, false);
	}

	void text_field::measure()
	{
		auto const width = std::max(m_min_width_px, m_texture.m_pos.x + m_texture.m_advance.x + padding.x + padding.z);
		auto const height = m_text.get_ft_font().get_line_height_px() + padding.y + padding.w;
		size = { width, height };
	}

	void text_field::input(input::input_event const& event)
	{
		namespace ie = input::input_events;

		if (std::holds_alternative<ie::mouse_motion>(event))
		{
			auto const& mm = std::get<ie::mouse_motion>(event);
			m_hovered = aabb{ position, size }.contains(mm.m_inv_y_position);
			m_pressed = m_pressed && m_hovered;
		}

		if (std::holds_alternative<ie::mouse_button>(event))
		{
			auto const& mb = std::get<ie::mouse_button>(event);
			
			if (mb.m_button == input::mouse_button::LEFT)
			{
				if (mb.m_value)
				{
					if (m_hovered)
					{
						m_pressed = true;
						m_focused = true;
						SDL_StartTextInput();
					}
					else
					{
						m_pressed = false;
						m_focused = false;
						SDL_StopTextInput();
					}
				}
				else
				{
					m_pressed = false;
				}
			}
		}

		if (m_focused)
		{
			if (std::holds_alternative<ie::text_input>(event))
			{
				auto const& t = std::get<ie::text_input>(event);
				insert_text(t.m_text.data(), false);
			}

			if (std::holds_alternative<ie::text_editing>(event))
			{
				auto const& c = std::get<ie::text_editing>(event);
				insert_text(c.m_text.data(), true);
			}

			if (std::holds_alternative<ie::keyboard_key>(event))
			{
				auto const& k = std::get<ie::keyboard_key>(event);

				using kt = bump::input::keyboard_key;

				if      (k.m_value && k.m_key == kt::ESCAPE) { m_focused = false; SDL_StopTextInput(); }
				else if (k.m_value && k.m_key == kt::ARROWLEFT) { move_caret(-1, k.m_mods.ctrl() ? cursor_mode::WORD : cursor_mode::CLUSTER, k.m_mods.shift()); }
				else if (k.m_value && k.m_key == kt::ARROWRIGHT) { move_caret(+1, k.m_mods.ctrl() ? cursor_mode::WORD : cursor_mode::CLUSTER, k.m_mods.shift()); }
				else if (k.m_value && k.m_key == kt::HOME) { set_caret(0, k.m_mods.shift(), false); }
				else if (k.m_value && k.m_key == kt::END) { set_caret(m_text.size(), k.m_mods.shift(), false); }
				else if (k.m_value && k.m_key == kt::BACKSPACE) { delete_codepoint(-1, k.m_mods.ctrl()); }
				else if (k.m_value && k.m_key == kt::DELETE) { delete_grapheme_cluster(+1, k.m_mods.ctrl()); }
			}
		}
	}

	void text_field::render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera)
	{
		auto const line_height_px = m_text.get_ft_font().get_line_height_px();
		auto const pad_px = vec{ padding.x, padding.y };

		// draw background
		ui_renderer.draw_rect(gl_renderer, camera, position, size, bg_color);

		// draw selection
		auto const selection_pos = vec{ std::min(m_caret_pos_px, m_selection_pos_px), 0 };
		auto const selection_size = vec{ std::max(m_caret_pos_px, m_selection_pos_px) - selection_pos.x, line_height_px };
		ui_renderer.draw_rect(gl_renderer, camera, position + pad_px + selection_pos, selection_size, selection_color);

		// draw composition
		auto const composition_pos = vec{ std::min(m_caret_pos_px, m_composition_pos_px), 0 };
		auto const composition_size = vec{ std::max(m_caret_pos_px, m_composition_pos_px) - composition_pos.x, line_height_px };
		ui_renderer.draw_rect(gl_renderer, camera, position + pad_px + composition_pos, composition_size, composition_color);

		// draw text
		ui_renderer.draw_text(gl_renderer, camera, position + pad_px, m_texture, line_height_px, color);

		// draw caret
		if (m_focused)
		{
			// note: caret size and y pos are kinda arbitrary
			auto const caret_pos = vec{ m_caret_pos_px, 0 };
			auto const caret_size = vec{ 2, line_height_px };
			ui_renderer.draw_rect(gl_renderer, camera, position + pad_px + caret_pos, caret_size, caret_color);
		}
	}

	void text_field::redraw_text()
	{
		m_texture = m_text.render();
	}
	
	void text_field::insert_text(std::string_view text, bool compose)
	{
		// erase any selected text
		if (selection_size() != 0)
		{
			m_text.erase(selection_start(), selection_size());
			set_caret(selection_start(), false, false);
		}

		// erase any composition text
		if (composition_size() != 0)
		{
			m_text.erase(composition_start(), composition_size());
			set_caret(composition_start(), false, false);
		}

		// insert text
		auto insertion_end = m_text.insert(m_caret, text);

		// move the caret to the end of the inserted text
		set_caret(insertion_end, false, compose);

		// update texture
		redraw_text();
	}

	void text_field::delete_grapheme_cluster(std::ptrdiff_t diff, bool word)
	{
		// no selection - select next word or cluster
		if (selection_size() == 0)
			move_caret(diff, word ? cursor_mode::WORD : cursor_mode::CLUSTER, true);

		// erase selected text
		m_text.erase(selection_start(), selection_size());
		set_caret(selection_start(), false, false);

		// update texture
		redraw_text();
	}
	
	void text_field::delete_codepoint(std::ptrdiff_t diff, bool word)
	{
		// no selection - select next word or cluster
		if (selection_size() == 0)
			move_caret(diff, word ? cursor_mode::WORD : cursor_mode::CODEPOINT, true);

		// erase selected text
		m_text.erase(selection_start(), selection_size());
		set_caret(selection_start(), false, false);

		// update texture
		redraw_text();
	}

} // bump::ui
