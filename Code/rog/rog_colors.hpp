#pragma once

#include <bump_math.hpp>

namespace rog
{
	
	namespace colors
	{
		
		auto constexpr black       = glm::vec3(0);
		auto constexpr dark_grey   = glm::vec3(0.25);
		auto constexpr grey        = glm::vec3(0.5);
		auto constexpr light_grey  = glm::vec3(0.75);
		auto constexpr white       = glm::vec3(1.f);
		auto constexpr orange      = glm::vec3{ 1.f, 0.5f, 0 };
		auto constexpr dark_red    = glm::vec3{ 0.25f, 0, 0 };
		auto constexpr red         = glm::vec3{ 0.75f, 0, 0 };
		auto constexpr light_red   = glm::vec3{ 1.f, 0, 0 };
		auto constexpr green       = glm::vec3{ 0, 0.5f, 0.25f };
		auto constexpr light_green = glm::vec3{ 0, 1.f, 0 };
		auto constexpr blue        = glm::vec3{ 0, 0, 1.f };
		auto constexpr light_blue  = glm::vec3{ 0, 1.f, 1.f };
		auto constexpr umber       = glm::vec3{ 0.5f, 0.25f, 0 };
		auto constexpr light_umber = glm::vec3{ 0.75f, 0.5f, 0.25f };
		auto constexpr violet      = glm::vec3{ 1.f, 0, 1.f };
		auto constexpr yellow      = glm::vec3{ 1.f, 1.f, 0 };
		
	} // colors
	
} // rog
