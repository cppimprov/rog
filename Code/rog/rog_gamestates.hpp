#pragma once

#include <bump_gamestate.hpp>

#include <cstdint>

namespace rog
{

	bump::gamestate gamestate_dungeon(bump::app& app, std::int32_t level_depth);

} // rog
