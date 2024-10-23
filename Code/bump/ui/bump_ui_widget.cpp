#include "bump_ui_widget.hpp"

#include "bump_narrow_cast.hpp"
#include "bump_render_text.hpp"

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
		m_ft_context(&ft_context),
		m_font(&font),
		m_text(text)
	{
		set_text(text);
	}

	void label::set_text(std::string const& text)
	{
		m_text = text;
		m_texture = render_text_to_gl_texture(*m_ft_context, *m_font, m_text);
	}

	void label::render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera)
	{
		ui_renderer.draw_rect(gl_renderer, camera, position, size, bg_color);
		ui_renderer.draw_text(gl_renderer, camera, position, m_texture, color);
	}

	label_button::label_button(font::ft_context const& ft_context, font::font_asset const& font, std::string const& text):
		m_ft_context(&ft_context),
		m_font(&font),
		m_text(text),
		m_hovered(false),
		m_pressed(false)
	{
		set_text(text);
	}

	void label_button::set_text(std::string const& text)
	{
		m_text = text;
		m_texture = render_text_to_gl_texture(*m_ft_context, *m_font, m_text);
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
		ui_renderer.draw_text(gl_renderer, camera, position, m_texture, color);
	}
	
	text_field::text_field(font::ft_context const& ft_context, font::font_asset const& font, std::string const& text):
		m_ft_context(&ft_context),
		m_font(&font),
		m_text(),
		m_hovered(false),
		m_pressed(false),
		m_focused(false),
		m_min_width_px(100),
		m_max_length(255),
		m_caret(0),
		m_selection(0)
	{
		insert_text(text);
	}

	void text_field::set_text(std::string const& text, bool select)
	{
		m_text = text;

		if (m_text.size() > m_max_length)
			m_text.resize(m_max_length);

		set_caret(0, false);
		set_caret(m_text.size(), select);

		redraw_text();
	}

	void text_field::set_max_length(std::size_t length)
	{
		m_max_length = length;

		if (m_text.size() > m_max_length)
			m_text.resize(m_max_length);

		m_caret = std::min(m_caret, m_text.size());
		m_selection = std::min(m_selection, m_text.size());
		
		redraw_text();
	}

	void text_field::set_caret(std::size_t pos, bool select)
	{
		m_caret = std::min(pos, m_text.size());

		if (!select)
			m_selection = m_caret;
	}

	void text_field::move_caret(std::ptrdiff_t diff, bool word, bool select)
	{
		// we have a selection, but we want to clear it and move the caret to the appropriate end
		if (selection_size() != 0 && !select)
		{
			set_caret(diff < 0 ? selection_start() : selection_end(), false);
			return;
		}

		auto constexpr find_word_boundary = [] (std::string const& text, std::size_t pos, std::ptrdiff_t diff)
		{
			if (text.empty())
				return std::size_t{ 0 };

			auto const begin = text.begin() + pos;
			auto const end = text.end();
			auto const rbegin = std::reverse_iterator(begin);
			auto const rend = text.rend();

			auto constexpr is_space = [] (char c) { return c == ' '; };

			auto const last_adj_whitespace = diff < 0 ?
				std::find_if_not(rbegin, rend, is_space).base() :
				std::find_if_not(begin, end, is_space);

			auto const first_space = diff < 0 ?
				std::find_if(std::reverse_iterator(last_adj_whitespace), rend, is_space).base() :
				std::find_if(last_adj_whitespace, end, is_space);
			
			return static_cast<std::size_t>(first_space - text.begin());
		};

		auto constexpr safeish_add = [] (std::size_t bound, std::size_t pos, std::ptrdiff_t diff)
		{
			return static_cast<std::size_t>(
				std::clamp(
					static_cast<std::ptrdiff_t>(pos) + diff, 
					std::ptrdiff_t{ 0 },
					static_cast<std::ptrdiff_t>(bound)));
		};

		auto caret_pos = word ?
			find_word_boundary(m_text, m_caret, diff) : 
			safeish_add(m_text.size(), m_caret, diff);
		
		set_caret(caret_pos, select);
	}

	void text_field::measure()
	{
		auto const width = std::max(m_min_width_px, m_texture.m_pos.x + m_texture.m_texture.get_size().x);
		auto const height = m_font->m_ft_font.get_line_height();
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
					}
					else
					{
						m_pressed = false;
						m_focused = false;
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
			if (std::holds_alternative<ie::typing>(event))
			{
				auto const& t = std::get<ie::typing>(event);

				insert_text(t.m_text.data());
			}

			if (std::holds_alternative<ie::keyboard_key>(event))
			{
				auto const& k = std::get<ie::keyboard_key>(event);

				using kt = bump::input::keyboard_key;

				if      (k.m_value && k.m_key == kt::ESCAPE) { m_focused = false; }
				else if (k.m_value && k.m_key == kt::ARROWLEFT) { move_caret(-1, k.m_mods.ctrl(), k.m_mods.shift()); }
				else if (k.m_value && k.m_key == kt::ARROWRIGHT) { move_caret(+1, k.m_mods.ctrl(), k.m_mods.shift()); }
				else if (k.m_value && k.m_key == kt::HOME) { set_caret(0, k.m_mods.shift()); }
				else if (k.m_value && k.m_key == kt::END) { set_caret(m_text.size(), k.m_mods.shift()); }
				else if (k.m_value && k.m_key == kt::BACKSPACE) { delete_text(-1, k.m_mods.ctrl()); }
				else if (k.m_value && k.m_key == kt::DELETE) { delete_text(+1, k.m_mods.ctrl()); }
			}
		}
	}

	void text_field::render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera)
	{
		ui_renderer.draw_rect(gl_renderer, camera, position, size, bg_color);
		ui_renderer.draw_text(gl_renderer, camera, position, m_texture, color);
	}
	
	void text_field::redraw_text()
	{
		m_texture = render_text_to_gl_texture(*m_ft_context, *m_font, m_text);
	}
	
	void text_field::insert_text(std::string const& text)
	{
		// erase any selected text
		if (selection_size() != 0)
		{
			m_text.erase(selection_start(), selection_size());
			set_caret(selection_start(), false);
		}

		// calculate available space
		auto const space = m_max_length - m_text.size();
		auto const len = std::min(space, text.size());

		// insert as much text as we can in the available space
		m_text.insert(m_caret, std::string_view(text.begin(), text.begin() + len));

		// move the caret to the end of the inserted text
		set_caret(m_caret + len, false);

		// update texture
		redraw_text();
	}

	void text_field::delete_text(std::ptrdiff_t diff, bool word)
	{
		// erase any selected text
		if (selection_size() != 0)
		{
			m_text.erase(selection_start(), selection_size());
			set_caret(selection_start(), false);
		}
		else
		{
			// move caret while selecting
			move_caret(diff, word, true);

			// erase selected text
			m_text.erase(selection_start(), selection_size());
			set_caret(selection_start(), false);
		}

		// update texture
		redraw_text();
	}

} // bump::ui
