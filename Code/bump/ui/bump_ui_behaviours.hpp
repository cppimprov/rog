#pragma once

#include "bump_ui_vec.hpp"

namespace bump::ui::behaviours
{

	// class null
	// {
	// protected:

	// 	void enable() { }
	// 	void disable() { }
	// 	// todo: input
	// 	// todo: update
	// };

	// class hover
	// {
	// public:

	// 	void enable() { m_enabled = true; }
	// 	void disable() { m_enabled = false; }

	// 	bool input(rect const& rect, bump::input::input_event const& event);
		
	// 	bool is_enabled() const { return m_enabled; }
	// 	bool is_hovered() const { return m_hovered; }

	// private:

	// 	bool m_enabled = true;
	// 	bool m_hovered = false;
	// };

	// template<class V>
	// class button
	// {
	// public:

	// 	explicit button(V& view): m_view(view), m_enabled(true), m_hovered(false), m_pressed(false), m_position(0), m_size(0) { }

	// 	void enable() { m_enabled = true; }
	// 	void disable() { m_enabled = false; }

	// 	void place(vec pos, vec size) { m_position = pos; m_size = size; }
	// 	// todo: input!

	// 	bool is_enabled() const { return m_enabled; }
	// 	bool is_hovered() const { return m_hovered; }
	// 	bool is_pressed() const { return m_pressed; }
		
	// 	std::function<void()> on_press;

	// private:

	// 	V& m_view;

	// 	bool m_enabled;
	// 	bool m_hovered;
	// 	bool m_pressed;
	// 	vec m_position, m_size;
	// };

} // bump::ui::behaviours
