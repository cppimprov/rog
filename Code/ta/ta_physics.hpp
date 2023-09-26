#pragma once

#include <box2d/box2d.h>
#include <glm/glm.hpp>

namespace ta
{

	enum collision_category : std::uint16_t
	{
		none =              0,
		player =       1 << 0,
		bullet =       1 << 1,
		powerup =      1 << 2,
		tile =         1 << 3,
		tile_wall =    1 << 4,
		tile_void  =   1 << 5,
		world_bounds = 1 << 6,
	};
	
	inline b2Vec2 to_b2_vec2(glm::vec2 v) { return b2Vec2(v.x, v.y); }
	inline glm::vec2 to_glm_vec2(b2Vec2 v) { return glm::vec2(v.x, v.y); }

} // ta
