#pragma once

#include "ta_sprite.hpp"

#include <bump_die.hpp>
#include <bump_gl.hpp>
#include <bump_grid.hpp>
#include <bump_math.hpp>
#include <bump_result.hpp>
#include <bump_timer.hpp>

#include <box2d/box2d.h>
#include <entt.hpp>

#include <array>
#include <map>
#include <optional>
#include <vector>

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

	enum class direction
	{
		up,
		down,
		left,
		right,
		up_left,
		up_right,
		down_left,
		down_right,
	};

	enum class powerup_type
	{
		player_heal,
		player_speed,
		player_reload_speed,
		bullet_bounce,
		bullet_damage,
		bullet_speed,
		COUNT,
	};

	inline b2Vec2 to_b2_vec2(glm::vec2 v) { return b2Vec2(v.x, v.y); }
	inline glm::vec2 to_glm_vec2(b2Vec2 v) { return glm::vec2(v.x, v.y); }

	std::optional<direction> get_input_dir(bool input_up, bool input_down, bool input_left, bool input_right);
	glm::vec2 dir_to_vec(direction dir);
	float dir_to_angle(direction dir);
	bool is_diagonal(direction dir);

	glm::vec3 get_powerup_color(powerup_type type);

	// PLAYERS:
	struct c_player_id
	{
		std::int16_t m_id = 0;
	};

	struct c_player_hp
	{
		std::uint32_t m_hp = 0;
	};

	struct c_player_graphics
	{
		glm::vec3 m_color = glm::vec3(1.f);
	};

	struct c_player_powerups
	{
		std::map<powerup_type, float> m_timers;
	};

	struct c_player_physics
	{
		b2Body* m_b2_body = nullptr;
	};

	struct c_player_movement
	{
		bool m_moving = false;
		direction m_direction = direction::right;
	};

	struct c_player_input
	{
		bool m_input_up = false;
		bool m_input_down = false;
		bool m_input_left = false;
		bool m_input_right = false;
		bool m_input_fire = false;
		bump::timer<> m_reload_timer;
	};

	entt::entity create_player(entt::registry& registry, b2World& b2_world, std::int16_t id, glm::vec2 position_px, glm::vec3 color);
	void destroy_player(entt::registry& registry, b2World& b2_world, entt::entity player);

	// BULLETS:
	struct c_bullet_owner_id
	{
		entt::entity m_owner_id = entt::null;
	};

	struct c_bullet_lifetime
	{
		float m_lifetime = 0.f;
	};

	struct c_bullet_physics
	{
		b2Body* m_b2_body = nullptr;
	};

	entt::entity create_bullet(entt::registry& registry, b2World& b2_world, std::uint32_t owner_id, std::int16_t owner_player_id, glm::vec2 position_px, glm::vec2 velocity_px);
	void destroy_bullet(entt::registry& registry, b2World& b2_world, entt::entity bullet);
	
	// POWERUPS:
	struct c_powerup_type
	{
		powerup_type m_type;
	};

	struct c_powerup_lifetime
	{
		float m_lifetime = 0.f;
	};

	struct c_powerup_physics
	{
		b2Body* m_b2_body = nullptr;
	};

	entt::entity create_powerup(entt::registry& registry, b2World& b2_world, powerup_type type, glm::vec2 position_px);
	void destroy_powerup(entt::registry& registry, b2World& b2_world, entt::entity powerup);

	// TILES:
	enum class tile_type : std::size_t
	{
		grass,
		road_ew,
		road_ns,
		road_cross,
		building,
		rubble,
		water,
	};

	std::uint16_t get_tile_collision_category(tile_type type);
	std::uint16_t get_tile_collision_mask(tile_type type);

	struct c_tile_type
	{
		tile_type m_type;
	};

	struct c_tile_physics
	{
		b2Body* m_b2_body = nullptr;
	};

	entt::entity create_tile(entt::registry& registry, b2World& b2_world, tile_type type, glm::vec2 position_px);

	struct world
	{
		entt::registry m_registry;

		std::vector<entt::entity> m_players;
		std::vector<entt::entity> m_bullets;
		std::vector<entt::entity> m_powerups;
		bump::grid<entt::entity, 2> m_tiles;
	};

	struct world_physics
	{
		b2World m_b2_world;
	};
	
	struct world_graphics
	{
		std::vector<bump::gl::texture_2d const*> m_tile_textures;

		ta::tile_renderable m_tile_renderable;
		ta::object_renderable m_tank_renderable, m_tank_renderable_diagonal;
		ta::object_renderable m_bullet_renderable;
		ta::object_renderable m_powerup_renderable;
	};

	void load_test_map(world& world, world_physics& world_physics);
	void set_world_bounds(world_physics& world_physics, glm::vec2 size_px);

	namespace globals
	{

		auto const player_hp = std::uint32_t{ 100 };
		auto const bullet_damage = std::uint32_t{ 10 };

		auto const player_speed = 200.f;
		auto const bullet_speed = 500.f;

		auto const powerup_duration = 8.f;
		auto const powerup_lifetime = 5.f;
		auto const bullet_lifetime = 3.f;

		auto const tile_radius = glm::vec2(32.f);
		auto const player_radius = glm::vec2(32.f);
		auto const bullet_radius = glm::vec2(4.f);
		auto const powerup_radius = glm::vec2(16.f);

		auto const b2_scale_factor = 0.1f;
		auto const b2_inv_scale_factor = 1.f / b2_scale_factor;

		auto const reload_time = std::chrono::milliseconds{ 200 };
		auto const powerup_spawn_time = std::chrono::seconds{ 10 };

	} // globals

	// namespace net
	// {
	//
	// 	enum class message_type : std::uint8_t { HELLO, GOODBYE, };
	//
	// 	using message_id_t = std::uint32_t;
	//
	// 	struct message
	// 	{
	// 		message_type m_type;
	// 		message_id_t m_id;
	// 		std::vector<std::uint8_t> m_data;
	// 	};
	//
	// } // net

} // ta
