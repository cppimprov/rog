#pragma once

#include "ta_direction.hpp"
#include "ta_physics.hpp"
#include "ta_renderable.hpp"

#include <bump_enet.hpp>
#include <bump_grid.hpp>
#include <bump_timer.hpp>

#include <entt.hpp>

namespace bump::sdl { class window; }
namespace bump::gl { class renderer; }

namespace ta
{

	// PLAYER:
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
		std::map<enum class powerup_type, float> m_timers;
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

	entt::entity create_player(entt::registry& registry, b2World& b2_world, glm::vec2 position_px, glm::vec3 color);
	void destroy_player(entt::registry& registry, b2World& b2_world, entt::entity player);

	// BULLET:
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

	entt::entity create_bullet(entt::registry& registry, b2World& b2_world, entt::entity owner_id, std::int16_t owner_player_group_index, glm::vec2 position_px, glm::vec2 velocity_px);
	void destroy_bullet(entt::registry& registry, b2World& b2_world, entt::entity bullet);

	// POWERUP:
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

	glm::vec3 get_powerup_color(powerup_type type);

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

	// TILE:
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

	entt::entity create_tile(entt::registry& registry, b2World& b2_world, tile_type type, glm::vec2 position_px, glm::vec2 radius_px);

	// PLAYER SLOT:
	struct player_slot
	{
		glm::vec3 m_color = glm::vec3(1.f);
		glm::vec2 m_start_pos_px = glm::vec2(0.f);
		entt::entity m_entity = entt::null;
		bump::enet::peer m_peer = bump::enet::peer();
	};

	// WORLD:
	struct world
	{
		// init:
		std::vector<player_slot> m_player_slots;

		// ecs:
		entt::registry m_registry;

		std::vector<entt::entity> m_players;
		std::vector<entt::entity> m_bullets;
		std::vector<entt::entity> m_powerups;
		bump::grid<entt::entity, 2> m_tiles;

		// physics:
		b2World m_b2_world;

		// graphics:
		std::vector<bump::gl::texture_2d const*> m_tile_textures;

		ta::tile_renderable m_tile_renderable;
		ta::object_renderable m_tank_renderable;
		ta::object_renderable m_tank_renderable_diagonal;
		ta::object_renderable m_bullet_renderable;
		ta::object_renderable m_powerup_renderable;
	};

	void load_test_map(world& world);
	void set_world_bounds(b2World& b2_world, glm::vec2 size_px);
	void render_world(bump::sdl::window const& window, bump::gl::renderer& renderer, world& world);

} // ta
