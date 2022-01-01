#pragma once

#include "rog_colors.hpp"
#include "rog_screen_cell.hpp"

#include <bump_grid.hpp>

#include <cstdint>

namespace rog
{

	struct feature
	{
		enum flags : std::uint64_t
		{
			NO_WALK = 1u << 1,
			NO_FLY  = 1u << 2,
		};
		
		screen::cell m_cell;
		flags m_flags;
	};

	namespace features
	{
		
		const auto wall =  feature{ { '#', colors::white, colors::black }, feature::flags::NO_WALK };
		const auto floor = feature{ { '.', colors::white, colors::black }, {} };

		const auto rock = feature{ { '.', colors::grey, colors::black }, {} };
		const auto mud = feature{ { '.', colors::umber, colors::black }, {} };
		const auto grass = feature{ { '.', colors::green, colors::black }, {} };
		const auto light_grass = feature{ { '.', colors::light_green, colors::black }, {} };

		const auto yellow_flower = feature{ { ';', colors::yellow, colors::black }, {} };
		const auto tall_grass = feature{ { ';', colors::green, colors::black }, {} };

		const auto tree = feature{ { '#', colors::green, colors::black }, {} };
		const auto dead_tree = feature{ { '#', colors::umber, colors::black }, {} };
		const auto light_tree = feature{ { '#', colors::light_green, colors::black }, {} };
		
	} // features

} // rog
