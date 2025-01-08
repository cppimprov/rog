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
#include "bump_ui_renderer.hpp"
#include "bump_ui_text_shape.hpp"
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
		virtual void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) = 0;
	};

	class quad : public widget_base
	{
	public:

		void measure() override { /* nothing to do - size is set directly */ }
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& ) override { }
		void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) override;

		glm::vec4 color = glm::vec4(1.f);
	};

	class textured_quad : public widget_base
	{
	public:

		textured_quad(gl::texture_2d const& texture): 
			m_texture(&texture) { }

		void set_texture(gl::texture_2d const& texture) { m_texture = &texture; }
		gl::texture_2d const* get_texture() const { return m_texture; }

		void measure() override { size = m_texture->get_size(); }
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& ) override { }
		void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) override;

		glm::vec4 color = glm::vec4(1.f);

	private:

		gl::texture_2d const* m_texture;
	};
	
	class label : public widget_base
	{
	public:

		label(font::ft_context const& ft_context, font::font_asset const& font, std::string const& text);

		void set_text(std::string const& text);
		void set_font(font::font_asset const& font) { m_text.set_font(font.m_ft_font, font.m_hb_font); }

		void measure() override;
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& ) override { }
		void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) override;

		glm::vec4 color = glm::vec4(1.f);
		glm::vec4 bg_color = glm::vec4(glm::vec3(0.2f), 1.f);
		margin_vec padding = margin_vec(2);

	private:
		
		void redraw_text();

		text_shape m_text;
		text_texture m_texture;
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

		void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) override
		{
			for (auto& c : children)
				if (c)
					c->render(ui_renderer, gl_renderer, camera);
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

		void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) override
		{
			for (auto& c : children)
				if (c)
					c->render(ui_renderer, gl_renderer, camera);
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

		void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) override
		{
			for (auto& c : children)
				if (c)
					c->render(ui_renderer, gl_renderer, camera);
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

		void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) override
		{
			for (auto& c : children)
				if (c)
					c->render(ui_renderer, gl_renderer, camera);
		}

		std::vector<std::shared_ptr<widget_base>> children;
	};
	
	class label_button : public widget_base
	{
	public:

		label_button(font::ft_context const& ft_context, font::font_asset const& font, std::string const& text);

		void set_text(std::string const& text);
		void set_font(font::font_asset const& font) { m_text.set_font(font.m_ft_font, font.m_hb_font); }

		void measure() override;
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& event) override;
		void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) override;

		glm::vec4 inactive_color = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
		glm::vec4 hover_color =    glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
		glm::vec4 press_color =    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 bg_color = glm::vec4(glm::vec3(0.2f), 1.f);
		margin_vec padding = margin_vec(2);
		std::function<void()> action;

	private:
		
		void redraw_text();

		text_shape m_text;
		text_texture m_texture;

		bool m_hovered;
		bool m_pressed;
	};
	
	class text_field : public widget_base
	{
	public:

		text_field(font::ft_context const& ft_context, font::font_asset const& font, std::string const& text);

		void set_text(std::string const& text, bool select);
		std::string const& get_text() const { return m_text.get(); }
		void set_font(font::font_asset const& font) { m_text.set_font(font.m_ft_font, font.m_hb_font); redraw_text(); }

		void set_min_width(vec::value_type min_width_px) { m_min_width_px = min_width_px; }
		vec::value_type get_min_width() const { return m_min_width_px; }

		void set_max_length(std::size_t length);
		std::size_t get_max_length() const { return m_text.get_max_length(); }

		void set_caret(std::size_t pos, bool select, bool compose);
		std::size_t get_caret() const { return m_caret; }
		
		enum class cursor_mode { WORD, CLUSTER, CODEPOINT };
		void move_caret(std::ptrdiff_t diff, cursor_mode mode, bool select);

		std::size_t selection_start() const { return std::min(m_caret, m_selection); }
		std::size_t selection_end() const { return std::max(m_caret, m_selection); }
		std::size_t selection_size() const { return selection_end() - selection_start(); }
		std::string_view get_selection() const { return std::string_view(m_text.get().begin() + selection_start(), m_text.get().begin() + selection_end()); }

		std::size_t composition_start() const { return std::min(m_caret, m_composition); }
		std::size_t composition_end() const { return std::max(m_caret, m_composition); }
		std::size_t composition_size() const { return composition_end() - composition_start(); }
		std::string_view get_composition() const { return std::string_view(m_text.get().begin() + composition_start(), m_text.get().begin() + composition_end()); }

		void measure() override;
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

		void input(input::input_event const& event) override;
		void render(ui::renderer const& ui_renderer, gl::renderer& gl_renderer, camera_matrices const& camera) override;

		glm::vec4 color = glm::vec4(1.f);
		glm::vec4 bg_color = { 0.2f, 0.2f, 0.2f, 1.f };
		glm::vec4 caret_color = { 1.f, 0.4f, 0.f, 1.f };
		glm::vec4 selection_color = { 1.f, 0.7f, 0.1f, 1.f };
		glm::vec4 composition_color = { 0.7f, 0.0f, 0.f, 1.f };
		margin_vec padding = margin_vec(10);

	private:

		void redraw_text();
		void insert_text(std::string_view text, bool compose);
		void delete_grapheme_cluster(std::ptrdiff_t diff, bool word);
		void delete_codepoint(std::ptrdiff_t diff, bool word);

		text_shape m_text;
		text_texture m_texture;

		bool m_hovered;
		bool m_pressed;
		bool m_focused;

		vec::value_type m_min_width_px;
		std::size_t m_caret;
		std::size_t m_selection;
		std::size_t m_composition;

		std::int32_t m_caret_pos_px;
		std::int32_t m_selection_pos_px;
		std::int32_t m_composition_pos_px;
	};

} // bump::ui
