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
			STAIRS_DOWN = 1u << 3,
			STAIRS_UP =   1u << 4,
		};
		
		screen::cell m_cell;
		flags m_flags;
	};

	namespace features
	{
		
		const auto empty = feature{ { ' ', colors::black, colors::black }, feature::flags::NO_WALK };
		const auto wall =  feature{ { '#', colors::white, colors::black }, feature::flags::NO_WALK };
		const auto floor = feature{ { '.', colors::white, colors::black }, {} };

		const auto stairs_down = feature{ { '>', colors::white, colors::black }, feature::flags::STAIRS_DOWN };
		const auto stairs_up =   feature{ { '<', colors::white, colors::black }, feature::flags::STAIRS_UP };

	} // features

} // rog
