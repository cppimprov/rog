#pragma once

#include "rog_direction.hpp"

#include <variant>

namespace rog
{
	
	namespace player_actions
	{

		struct move { direction m_dir; };
		struct use_stairs { stairs_direction m_dir; };

	} // actions

	using player_action = std::variant
	<
		player_actions::move,
		player_actions::use_stairs
	>;
	
} // rog
