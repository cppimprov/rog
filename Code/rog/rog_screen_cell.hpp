#pragma once

#include <bump_math.hpp>

#include <cstdint>

namespace rog
{
	
	namespace screen
	{
		
		struct cell
		{
			std::uint8_t m_value;
			glm::vec3 m_fg;
			glm::vec3 m_bg;
		};
		
	} // screen
	
} // rog
