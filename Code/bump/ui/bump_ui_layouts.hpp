#pragma once

#include "bump_grid.hpp"
#include "bump_narrow_cast.hpp"
#include "bump_ui_vec.hpp"
#include "bump_ui_widget.hpp"

#include <vector>

namespace bump::ui::layouts
{

	// class vector_v
	// {
	// public:

	// 	vector_v(): spacing(0) { }

	// 	void layout_measure_children() { for (auto& c : children) c->layout_measure(); }
		
	// 	vec layout_measure()
	// 	{
	// 		auto size = vec(0);

	// 		for (auto const& c : children)
	// 		{
	// 			auto const s = c->get_total_size();
	// 			size.x = std::max(size.x, s.x);
	// 			size.y += s.y;
	// 		}
			
	// 		if (children.size() > 1)
	// 			size.y += spacing * (narrow_cast<vec::value_type>(children.size()) - 1);
			
	// 		return size;
	// 	}

	// 	void layout_solve_children(vec pos, vec size)
	// 	{
	// 		auto offset = pos;

	// 		for (auto const& c : children)
	// 		{
	// 			auto const s = c->get_total_size();
	// 			c->layout_solve(offset, { size.x, s.y });
	// 			offset.y += s.y + spacing;
	// 		}
	// 	}

	// 	void render_children(/* ... */) { for (auto& c : children) c->render(); }

	// 	vec::value_type spacing;
	// 	std::vector<std::shared_ptr<widget_base>> children;
	// };

	// class vector_h
	// {
	// public:

	// 	vector_h(): spacing(0) { }

	// 	void layout_measure_children() { for (auto& c : children) c->measure(); }

	// 	vec layout_measure()
	// 	{
	// 		auto size = vec(0);

	// 		for (auto const& c : children)
	// 		{
	// 			auto const s = c->get_total_size();
	// 			size.x += s.x;
	// 			size.y = std::max(size.y, s.y);
	// 		}
			
	// 		if (children.size() > 1)
	// 			size.x += spacing * (narrow_cast<vec::value_type>(children.size()) - 1);
			
	// 		return size;
	// 	}

	// 	void layout_solve_children(vec pos, vec size)
	// 	{
	// 		auto offset = pos;

	// 		for (auto const& c : children)
	// 		{
	// 			auto const s = c->get_total_size();
	// 			c.solve(offset, { s.x, size.x });
	// 			offset.x += s.x + spacing;
	// 		}
	// 	}
		
	// 	void render_children(/* ... */) { for (auto& c : children) c->render(); }

	// 	vec::value_type spacing;
	// 	std::vector<std::shared_ptr<widget_base>> children;
	// };

	// class grid
	// {
	// public:

	// 	grid(): spacing(0) { }

	// 	void layout_measure_children() { for (auto& c : children) c->measure(); }

	// 	vec layout_measure()
	// 	{
	// 		using size_t = grid2<widget>::size_type;

	// 		auto const grid_size = children.extents();

	// 		auto max_cols = std::vector<vec::value_type>(grid_size.x, 0);
	// 		auto max_rows = std::vector<vec::value_type>(grid_size.y, 0);

	// 		for (auto x = size_t{ 0 }; x != grid_size.x; ++x)
	// 		{
	// 			for (auto y = size_t{ 0 }; y != grid_size.y; ++y)
	// 			{
	// 				auto const s = children.at({ x, y }).get_total_size();
	// 				max_cols[x] = std::max(max_cols[x], s.x);
	// 				max_cols[y] = std::max(max_cols[y], s.y);
	// 			}
	// 		}

	// 		auto size = vec(0);
			
	// 		for (auto const& x : max_cols) size.x += x;
	// 		if (grid_size.x > 1) size.x += spacing.x * (narrow_cast<vec::value_type>(grid_size.x) - 1);

	// 		for (auto const& y : max_rows) size.y += y;
	// 		if (grid_size.y > 1) size.y += spacing.y * (narrow_cast<vec::value_type>(grid_size.y) - 1);
			
	// 		return size;
	// 	}

	// 	void layout_solve_children(vec pos, vec )
	// 	{
	// 		using size_t = grid2<widget>::size_type;

	// 		auto const grid_size = children.extents();

	// 		auto max_cols = std::vector<vec::value_type>(grid_size.x, 0);
	// 		auto max_rows = std::vector<vec::value_type>(grid_size.y, 0);

	// 		for (auto x = size_t{ 0 }; x != grid_size.x; ++x)
	// 		{
	// 			for (auto y = size_t{ 0 }; y != grid_size.y; ++y)
	// 			{
	// 				auto const s = children.at({ x, y }).get_total_size();
	// 				max_cols[x] = std::max(max_cols[x], s.x);
	// 				max_cols[y] = std::max(max_cols[y], s.y);
	// 			}
	// 		}

	// 		auto offset = pos;

	// 		for (auto x = size_t{ 0 }; x != grid_size.x; ++x)
	// 		{
	// 			offset.y = pos.y;

	// 			for (auto y = size_t{ 0 }; y != grid_size.y; ++y)
	// 			{
	// 				children.at({ x, y }).solve(offset, { max_cols[x], max_rows[y] });
	// 				offset.y += max_rows[y] + spacing.y;
	// 			}

	// 			offset.x += max_cols[x] + spacing.x;
	// 		}
	// 	}
		
	// 	void render_children(/* ... */) { for (auto& c : children) c->render(); }

	// 	vec spacing;
	// 	bump::grid2<widget> children;
	// };

	// todo (maybe): overlay
	// todo (maybe): canvas
	// todo (maybe): area

} // bump::ui::layouts
