#include "ta_state.hpp"

#include <ta_world.hpp>
#include <ta_globals.hpp>

#include <bump_app.hpp>
#include <bump_gamestate.hpp>
#include <bump_log.hpp>

namespace ta
{

	bump::gamestate loading(bump::app& app)
	{
		bump::log_info("loading()");

		auto world = std::unique_ptr<ta::world>(new ta::world
		{
			.m_player_slots =
			{
				{ glm::vec3(1.f, 0.8f, 0.3f), globals::player_radius * glm::vec2{ 1.f, 8.f }, entt::null, { } },
				{ glm::vec3(1.f, 0.f, 0.f), globals::player_radius * glm::vec2{ 5.f, 3.f }, entt::null, { } },
				{ glm::vec3(0.f, 0.9f, 0.f), globals::player_radius * glm::vec2{ 7.f, 3.f }, entt::null, { } },
				{ glm::vec3(0.2f, 0.2f, 1.f), globals::player_radius * glm::vec2{ 9.f, 3.f }, entt::null, { } },
			},

			.m_b2_world = b2World{ b2Vec2{ 0.f, 0.f } },

			.m_tile_textures =
			{
				&app.m_assets.m_textures_2d["grass"],
				&app.m_assets.m_textures_2d["road_ew"],
				&app.m_assets.m_textures_2d["road_ns"],
				&app.m_assets.m_textures_2d["road_cross"],
				&app.m_assets.m_textures_2d["building"],
				&app.m_assets.m_textures_2d["rubble"],
				&app.m_assets.m_textures_2d["water"],
			},

			.m_tile_renderable = ta::tile_renderable(app.m_assets.m_shaders["sprite"]),
			.m_tank_renderable = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["tank"], app.m_assets.m_textures_2d["tank_accent"]),
			.m_tank_renderable_diagonal = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["tank_diagonal"], app.m_assets.m_textures_2d["tank_accent_diagonal"]),
			.m_bullet_renderable = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["bullet"], app.m_assets.m_textures_2d["bullet_accent"]),
			.m_powerup_renderable = ta::object_renderable(app.m_assets.m_shaders["sprite_accent"], app.m_assets.m_textures_2d["powerup"], app.m_assets.m_textures_2d["powerup_accent"]),
		});

		load_test_map(*world);
		set_world_bounds(world->m_b2_world, glm::vec2(world->m_tiles.extents()) * globals::tile_radius * 2.f);

		return { [&, world = std::move(world)] (bump::app& app) mutable { return connect_to_server(app, std::move(world)); } };
	}

} // ta
