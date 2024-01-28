#pragma once

#include <bump_math.hpp>
#include <bump_time.hpp>

#include <cstdint>

namespace ta
{

	namespace globals
	{

		auto const player_hp = std::uint32_t{ 100 };
		auto const bullet_damage = std::uint32_t{ 10 };

		auto const player_speed = 200.f;
		auto const bullet_speed = 500.f;

		auto const powerup_duration = 8.f;
		auto const powerup_lifetime = 5.f;
		auto const bullet_lifetime = 3.f;

		auto const powerup_player_heal_hp = std::uint32_t{ 50 };
		auto const powerup_player_speed_multiplier = 1.5f;
		auto const powerup_player_reload_speed = std::chrono::milliseconds{ 100 };
		auto const powerup_bullet_damage_multiplier = 2.f;
		auto const powerup_bullet_speed_multiplier = 1.75f;

		auto const tile_radius = glm::vec2(32.f);
		auto const player_radius = glm::vec2(32.f);
		auto const bullet_radius = glm::vec2(4.f);
		auto const powerup_radius = glm::vec2(16.f);

		auto const b2_scale_factor = 0.1f;
		auto const b2_inv_scale_factor = 1.f / b2_scale_factor;

		auto const reload_time = std::chrono::milliseconds{ 200 };
		auto const powerup_spawn_time = std::chrono::seconds{ 3 };

		auto const server_update_tick_rate = std::chrono::duration_cast<bump::high_res_duration_t>(std::chrono::milliseconds{ 32 });
		auto const server_update_tick_rate_f = bump::high_res_duration_to_seconds(server_update_tick_rate);

		auto const client_update_tick_rate = std::chrono::duration_cast<bump::high_res_duration_t>(std::chrono::milliseconds{ 16 });
		auto const client_update_tick_rate_f = bump::high_res_duration_to_seconds(client_update_tick_rate);

	} // globals

} // ta
