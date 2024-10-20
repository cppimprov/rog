#pragma once

#include "bump_aabb.hpp"
#include "bump_camera.hpp"
#include "bump_font.hpp"
#include "bump_gl.hpp"
#include "bump_grid.hpp"
#include "bump_input.hpp"
#include "bump_log.hpp"
#include "bump_render_text.hpp"
#include "bump_time.hpp"
#include "bump_ui_box.hpp"
#include "bump_ui_vec.hpp"

#include <iostream>
#include <memory>
#include <string>

namespace bump::ui
{
	
	class widget_base : public box
	{
	public:

		virtual ~widget_base() { }

		// `measure` should set the `size` of the box base class based on its content.
		// `measure` must also be called on the widget's children here.
		virtual void measure() = 0;

		// `place` should calculate the absolute position of the widget. it may adjust the size if necessary.
		// `place` must also be called on the widget's children here.
		virtual void place(vec cell_pos, vec cell_size) = 0;

		virtual void input(input::input_event const& event) = 0;

		//virtual void update(duration_t dt, sdl::input_handler const& input) = 0;
		virtual void render(gl::renderer& renderer, camera_matrices const& camera) = 0;
	};

	class quad : public widget_base
	{
	public:

		explicit quad(gl::shader_program const& shader);

		void measure() override { /* nothing to do - size is set directly */ }
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& ) override { }
		void render(gl::renderer& renderer, camera_matrices const& camera) override;

		glm::vec4 color = glm::vec4(1.f);

	private:
		
		gl::shader_program const* m_shader;
		
		GLint m_in_VertexPosition;
		GLint m_u_Position;
		GLint m_u_Size;
		GLint m_u_Color;
		GLint m_u_MVP;

		gl::buffer m_vertex_buffer;
		gl::vertex_array m_vertex_array;
	};

	class textured_quad : public widget_base
	{
	public:

		textured_quad(gl::shader_program const& shader, gl::texture_2d const& texture);

		void measure() override { size = m_texture->get_size(); }
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& ) override { }
		void render(gl::renderer& renderer, camera_matrices const& camera) override;

		glm::vec4 color = glm::vec4(1.f);

	private:
		
		gl::shader_program const* m_shader;
		gl::texture_2d const* m_texture;
		
		GLint m_in_VertexPosition;
		GLint m_u_Position;
		GLint m_u_Size;
		GLint m_u_Color;
		GLint m_u_Texture;
		GLint m_u_MVP;

		gl::buffer m_vertex_buffer;
		gl::vertex_array m_vertex_array;
	};

	class label : public widget_base
	{
	public:

		label(gl::shader_program const& shader, font::ft_context const& ft_context, font::font_asset const& font, std::string const& text);

		void set_text(std::string const& text);
		void set_font(font::font_asset const& font) { m_font = &font; set_text(m_text); }

		void measure() override { size = m_texture.m_pos + m_texture.m_texture.get_size(); }
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& ) override { }
		void render(gl::renderer& renderer, camera_matrices const& camera) override;

		glm::vec4 color = glm::vec4(1.f);

	private:
		
		gl::shader_program const* m_shader;
		font::ft_context const* m_ft_context;
		font::font_asset const* m_font;
		std::string m_text;
		text_texture m_texture;
		
		GLint m_in_VertexPosition;
		GLint m_u_Position;
		GLint m_u_Size;
		GLint m_u_Offset;
		GLint m_u_Color;
		GLint m_u_Texture;
		GLint m_u_MVP;

		gl::buffer m_vertex_buffer;
		gl::vertex_array m_vertex_array;
	};

	class vector_v : public widget_base
	{
	public:

		void measure() override
		{
			for (auto& c : children)
				if (c)
					c->measure();

			auto content_size = vec(0);

			for (auto const& c : children)
			{
				auto const s = c ? c->get_total_size() : vec(0);
				content_size.x = std::max(content_size.x, s.x);
				content_size.y += s.y;
			}

			if (children.size() > 1)
				content_size.y += spacing * (narrow_cast<vec::value_type>(children.size()) - 1);
			
			box_measure(content_size);
		}

		void place(vec cell_pos, vec cell_size) override
		{
			box_place(cell_pos, cell_size);

			auto offset = position;

			for (auto const& c : children)
			{
				auto const s = c ? c->get_total_size() : vec(0);
				c->place(offset, { size.x, s.y });
				offset.y += s.y + spacing;
			}
		}

		void input(input::input_event const& event) override
		{
			for (auto& c : children)
				if (c)
					c->input(event);
		}

		void render(gl::renderer& renderer, camera_matrices const& camera) override
		{
			for (auto& c : children)
				if (c)
					c->render(renderer, camera);
		}

		vec::value_type spacing;
		std::vector<std::shared_ptr<widget_base>> children;
	};

	class vector_h : public widget_base
	{
	public:

		void measure() override
		{
			for (auto& c : children)
				c->measure();

			auto content_size = vec(0);

			for (auto const& c : children)
			{
				auto const s = c ? c->get_total_size() : vec(0);
				content_size.y = std::max(content_size.y, s.y);
				content_size.x += s.x;
			}

			if (children.size() > 1)
				content_size.x += spacing * (narrow_cast<vec::value_type>(children.size()) - 1);
			
			box_measure(content_size);
		}

		void place(vec cell_pos, vec cell_size) override
		{
			box_place(cell_pos, cell_size);

			auto offset = position;

			for (auto const& c : children)
			{
				auto const s = c ? c->get_total_size() : vec(0);
				c->place(offset, { size.y, s.x });
				offset.x += s.x + spacing;
			}
		}

		void input(input::input_event const& event) override
		{
			for (auto& c : children)
				if (c)
					c->input(event);
		}

		void render(gl::renderer& renderer, camera_matrices const& camera) override
		{
			for (auto& c : children)
				if (c)
					c->render(renderer, camera);
		}

		vec::value_type spacing;
		std::vector<std::shared_ptr<widget_base>> children;
	};

	class grid : public widget_base
	{
	public:

		void measure() override
		{
			for (auto& c : children)
				c->measure();

			using size_t = grid2<std::shared_ptr<widget_base>>::size_type;

			auto const grid_size = children.extents();

			auto max_cols = std::vector<vec::value_type>(grid_size.x, 0);
			auto max_rows = std::vector<vec::value_type>(grid_size.y, 0);

			for (auto x = size_t{ 0 }; x != grid_size.x; ++x)
			{
				for (auto y = size_t{ 0 }; y != grid_size.y; ++y)
				{
					auto const& c = children.at({ x, y });
					auto const s = c ? c->get_total_size() : vec(0);
					max_cols[x] = std::max(max_cols[x], s.x);
					max_rows[y] = std::max(max_rows[y], s.y);
				}
			}

			auto content_size = vec(0);

			for (auto const& x : max_cols) content_size.x += x;
			if (grid_size.x > 1) content_size.x += spacing.x * (narrow_cast<vec::value_type>(grid_size.x) - 1);

			for (auto const& y : max_rows) content_size.y += y;
			if (grid_size.y > 1) content_size.y += spacing.y * (narrow_cast<vec::value_type>(grid_size.y) - 1);
			
			box_measure(content_size);
		}

		void place(vec cell_pos, vec cell_size) override
		{
			box_place(cell_pos, cell_size);

			using size_t = grid2<std::shared_ptr<widget_base>>::size_type;

			auto const grid_size = children.extents();

			auto max_cols = std::vector<vec::value_type>(grid_size.x, 0);
			auto max_rows = std::vector<vec::value_type>(grid_size.y, 0);

			for (auto x = size_t{ 0 }; x != grid_size.x; ++x)
			{
				for (auto y = size_t{ 0 }; y != grid_size.y; ++y)
				{
					auto const& c = children.at({ x, y });
					auto const s = c ? c->get_total_size() : vec(0);
					max_cols[x] = std::max(max_cols[x], s.x);
					max_rows[y] = std::max(max_rows[y], s.y);
				}
			}

			auto offset = cell_pos;
			
			for (auto x = size_t{ 0 }; x != grid_size.x; ++x)
			{
				offset.y = cell_pos.y;

				for (auto y = size_t{ 0 }; y != grid_size.y; ++y)
				{
					auto const& c = children.at({ x, y });
					if (c) children.at({ x, y })->place(offset, { max_cols[x], max_rows[y] });
					offset.y += max_rows[y] + spacing.y;
				}

				offset.x += max_cols[x] + spacing.x;
			}
		}

		void input(input::input_event const& event) override
		{
			for (auto& c : children)
				if (c)
					c->input(event);
		}

		void render(gl::renderer& renderer, camera_matrices const& camera) override
		{
			for (auto& c : children)
				if (c)
					c->render(renderer, camera);
		}

		vec spacing;
		grid2<std::shared_ptr<widget_base>> children;
	};

	class canvas : public widget_base
	{
	public:

		void measure() override
		{
			for (auto& c : children)
				if (c)
					c->measure();
			
			auto content_size = vec(0);

			for (auto const& c : children)
			{
				auto const s = c ? c->get_total_size() : vec(0);
				content_size = glm::max(content_size, s);
			}

			box_measure(content_size);
		}

		void place(vec cell_pos, vec cell_size) override
		{
			box_place(cell_pos, cell_size);

			auto offset = position;

			for (auto& c : children)
				if (c)
					c->place(offset, size);
		}

		void input(input::input_event const& event) override
		{
			for (auto& c : children)
				if (c)
					c->input(event);
		}

		void render(gl::renderer& renderer, camera_matrices const& camera) override
		{
			for (auto& c : children)
				if (c)
					c->render(renderer, camera);
		}

		std::vector<std::shared_ptr<widget_base>> children;
	};
	
	class label_button : public widget_base
	{
	public:

		label_button(gl::shader_program const& shader, font::ft_context const& ft_context, font::font_asset const& font, std::string const& text);

		void set_text(std::string const& text);
		void set_font(font::font_asset const& font) { m_font = &font; set_text(m_text); }

		void measure() override { size = m_texture.m_pos + m_texture.m_texture.get_size(); }
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& event) override
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

		void render(gl::renderer& renderer, camera_matrices const& camera) override;

		glm::vec4 inactive_color = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
		glm::vec4 hover_color =    glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
		glm::vec4 press_color =    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		std::function<void()> action;

	private:
		
		gl::shader_program const* m_shader;
		font::ft_context const* m_ft_context;
		font::font_asset const* m_font;
		std::string m_text;
		text_texture m_texture;
		
		GLint m_in_VertexPosition;
		GLint m_u_Position;
		GLint m_u_Size;
		GLint m_u_Offset;
		GLint m_u_Color;
		GLint m_u_Texture;
		GLint m_u_MVP;

		gl::buffer m_vertex_buffer;
		gl::vertex_array m_vertex_array;
		
		bool m_hovered;
		bool m_pressed;
	};
	
	class text_field : public widget_base
	{
	public:

		text_field(gl::shader_program const& shader, font::ft_context const& ft_context, font::font_asset const& font, std::string const& text);

		void set_text(std::string const& text);
		std::string const& get_text() const { return m_text; }
		void set_font(font::font_asset const& font) { m_font = &font; set_text(m_text); }

		void set_max_length(std::size_t length);
		std::size_t get_max_length() const { return m_max_length; }

		void set_caret(std::size_t pos, bool select);
		std::size_t get_caret() const { return m_caret; }
		
		void move_caret(std::ptrdiff_t diff, bool word, bool select);

		std::size_t selection_start() const { return std::min(m_caret, m_selection); }
		std::size_t selection_end() const { return std::max(m_caret, m_selection); }
		std::size_t selection_size() const { return selection_end() - selection_start(); }
		std::string_view get_selection() const { return std::string_view(m_text.begin() + selection_start(), m_text.begin() + selection_end()); }

		void measure() override { size = m_texture.m_pos + m_texture.m_texture.get_size(); }
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& event) override;
		void render(gl::renderer& renderer, camera_matrices const& camera) override;

		glm::vec4 color = glm::vec4(1.f);

	private:

		void redraw_text();
		void insert_text(std::string const& text);
		void delete_text(std::ptrdiff_t diff, bool word);
		
		gl::shader_program const* m_shader;
		font::ft_context const* m_ft_context;
		font::font_asset const* m_font;
		std::string m_text;
		text_texture m_texture;
		
		GLint m_in_VertexPosition;
		GLint m_u_Position;
		GLint m_u_Size;
		GLint m_u_Offset;
		GLint m_u_Color;
		GLint m_u_Texture;
		GLint m_u_MVP;

		gl::buffer m_vertex_buffer;
		gl::vertex_array m_vertex_array;
		
		bool m_hovered;
		bool m_pressed;
		bool m_focused;

		std::size_t m_max_length;
		std::size_t m_caret;
		std::size_t m_selection;
	};

	// text_field todo:
		// add ui_text_field shader
		// render caret / selection

} // bump::ui
