#pragma once

#include "bump_camera.hpp"
#include "bump_gl.hpp"
#include "bump_time.hpp"
#include "bump_ui_box.hpp"
#include "bump_ui_vec.hpp"

#include <memory>

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

		//virtual void input(input::input_event const& event) = 0;
		//virtual void update(duration_t dt, sdl::input_handler const& input) = 0;
		virtual void render(gl::renderer& renderer, camera_matrices const& camera) = 0;
	};

	class quad : public widget_base
	{
	public:

		quad(gl::shader_program const& shader);

		void measure() override { /* nothing to do - size is set directly */ }
		void place(vec cell_pos, vec cell_size) override { box_place(cell_pos, cell_size); }

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

	class vector_v : public widget_base
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

			auto offset = cell_pos;

			for (auto const& c : children)
			{
				auto const s = c ? c->get_total_size() : vec(0);
				c->place(offset, { size.x, s.y });
				offset.y += s.y + spacing;
			}
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

} // bump::ui
