#pragma once

#include "bump_ui_vec.hpp"

namespace bump::ui
{

	enum class origin
	{
		left = 0, top = 0,
		center = 1,
		right = 2, bottom = 2, 
	};

	using origin_vec = glm::vec<2, origin, glm::defaultp>;

	enum class fill
	{
		shrink,
		fixed,
		expand,
	};

	using fill_vec = glm::vec<2, fill, glm::defaultp>;

	struct box
	{
		// n.b. margin is not included in size
		// n.b. position is absolute (i.e. relative to the screen, not the parent widget)
		// n.b. position is the bottom(?)-left corner of the widget

		origin_vec origin = origin_vec(origin::left);
		fill_vec fill = fill_vec(fill::shrink);
		vec margin_pre = vec(0);
		vec margin_post = vec(0);
		vec size = vec(0);
		vec position = vec(0);

		vec get_total_size() const { return margin_pre + size + margin_post; }

	protected:

		void box_measure(vec measured_size)
		{
			if (fill.x == fill::shrink) size.x = measured_size.x;
			if (fill.y == fill::shrink) size.y = measured_size.y;
		}

		void box_place(vec cell_pos, vec cell_size)
		{
			// set size to fill the space in the cell
			// todo: could end up with a negative size here? do we need to clamp the output or something?
			if (fill.x == fill::expand) size.x = cell_size.x - (margin_pre.x + margin_post.x);
			if (fill.y == fill::expand) size.y = cell_size.y - (margin_pre.y + margin_post.y);

			// set the position from the origin and size
			switch (origin.x)
			{
			case origin::left:   position.x = cell_pos.x + margin_pre.x; break;
			case origin::center: position.x = cell_pos.x + (cell_size.x + margin_pre.x - size.x) / vec::value_type{ 2 }; break;
			case origin::right:  position.x = cell_pos.x + cell_size.x - (size.x + margin_post.x); break;
			}
			switch (origin.y)
			{
			case origin::left:   position.y = cell_pos.y + margin_pre.y; break;
			case origin::center: position.y = cell_pos.y + (cell_size.y + margin_pre.y - size.y) / vec::value_type{ 2 }; break;
			case origin::right:  position.y = cell_pos.y + cell_size.y - (size.y + margin_post.y); break;
			}
		}
	};

} // bump::ui
