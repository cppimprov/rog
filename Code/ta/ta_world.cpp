#include "ta_world.hpp"

#include "ta_globals.hpp"

#include <bump_gl_renderer.hpp>
#include <bump_sdl_window.hpp>
#include <bump_transform.hpp>

#include <ranges>

namespace ta
{

	// PLAYER:
	entt::entity create_player(entt::registry& registry, b2World& b2_world, std::uint8_t slot_index, glm::vec2 position_px, glm::vec3 color)
	{
		auto const b2_position = to_b2_vec2(globals::b2_scale_factor * position_px);

		auto body_def = b2BodyDef();
		body_def.type = b2_dynamicBody;
		body_def.position = b2_position;
		body_def.fixedRotation = true;

		auto const body = b2_world.CreateBody(&body_def);

		auto const b2_radius = globals::b2_scale_factor * globals::player_radius;

		auto shape = b2CircleShape();
		shape.m_radius = b2_radius.x;

		auto fixture_def = b2FixtureDef();
		fixture_def.shape = &shape;
		fixture_def.density = 1.f;
		fixture_def.friction = 0.f;
		fixture_def.restitution = 0.f;
		fixture_def.filter.categoryBits = collision_category::player;
		fixture_def.filter.maskBits = 
			collision_category::bullet | 
			collision_category::powerup | 
			collision_category::tile_wall | 
			collision_category::tile_void | 
			collision_category::world_bounds;

		static auto id = std::int16_t{ 0 };
		fixture_def.filter.groupIndex = --id;

		body->CreateFixture(&fixture_def);

		auto const entity = registry.create();

		registry.emplace<c_player_slot_index>(entity, slot_index);
		registry.emplace<c_player_hp>(entity, globals::player_hp);
		registry.emplace<c_player_graphics>(entity, color);
		registry.emplace<c_player_powerups>(entity);
		registry.emplace<c_player_physics>(entity, body);
		registry.emplace<c_player_movement>(entity);
		registry.emplace<c_player_reload>(entity);

		body->SetUserData(reinterpret_cast<void*>(entity));

		return entity;
	}

	void destroy_player(entt::registry& registry, b2World& b2_world, entt::entity player)
	{
		auto const& pp = registry.get<c_player_physics>(player);
		b2_world.DestroyBody(pp.m_b2_body);
		registry.destroy(player);
	}

	// BULLET:
	entt::entity create_bullet(entt::registry& registry, b2World& b2_world, std::uint32_t id, entt::entity owner_id, std::int16_t owner_player_group_index, glm::vec2 position_px, glm::vec2 velocity_px)
	{
		auto const b2_position = to_b2_vec2(globals::b2_scale_factor * position_px);
		auto const b2_velocity = to_b2_vec2(globals::b2_scale_factor * velocity_px);

		auto body_def = b2BodyDef();
		body_def.type = b2_dynamicBody;
		body_def.position = b2_position;
		body_def.linearVelocity = b2_velocity;
		body_def.fixedRotation = true;

		auto const body = b2_world.CreateBody(&body_def);

		auto const b2_radius = globals::b2_scale_factor * globals::bullet_radius;

		auto shape = b2CircleShape();
		shape.m_radius = b2_radius.x;

		auto fixture_def = b2FixtureDef();
		fixture_def.shape = &shape;
		fixture_def.density = 1.f;
		fixture_def.friction = 0.f;
		fixture_def.restitution = 1.f;
		fixture_def.filter.categoryBits = collision_category::bullet;
		fixture_def.filter.maskBits = 
			collision_category::player | 
			collision_category::tile_wall | 
			collision_category::world_bounds;
		fixture_def.filter.groupIndex = owner_player_group_index;
		
		body->CreateFixture(&fixture_def);

		auto const entity = registry.create();

		registry.emplace<c_bullet_id>(entity, id);
		registry.emplace<c_bullet_owner_id>(entity, owner_id);
		registry.emplace<c_bullet_lifetime>(entity, globals::bullet_lifetime);
		registry.emplace<c_bullet_physics>(entity, body);

		body->SetUserData(reinterpret_cast<void*>(entity));

		return entity;
	}

	void destroy_bullet(entt::registry& registry, b2World& b2_world, entt::entity bullet)
	{
		auto const& bp = registry.get<c_bullet_physics>(bullet);
		b2_world.DestroyBody(bp.m_b2_body);
		registry.destroy(bullet);
	}

	// POWERUP:
	glm::vec3 get_powerup_color(powerup_type type)
	{
		switch (type)
		{
		case powerup_type::player_heal:         return glm::vec3(0.0f, 0.95f, 0.0f);
		case powerup_type::player_speed:        return glm::vec3(0.5f, 0.5f, 0.95f);
		case powerup_type::player_reload_speed: return glm::vec3(0.8f, 0.8f, 0.2f);
		case powerup_type::bullet_bounce:       return glm::vec3(0.1f, 0.1f, 0.1f);
		case powerup_type::bullet_damage:       return glm::vec3(0.95f, 0.0f, 0.0f);
		case powerup_type::bullet_speed:        return glm::vec3(0.8f, 0.8f, 0.8f);
		}

		bump::die();
	}
	
	entt::entity create_powerup(entt::registry& registry, b2World& b2_world, powerup_type type, std::uint32_t id, glm::vec2 position_px)
	{
		auto const b2_position = to_b2_vec2(globals::b2_scale_factor * position_px);

		auto body_def = b2BodyDef();
		body_def.type = b2_staticBody;
		body_def.position = b2_position;

		auto const body = b2_world.CreateBody(&body_def);

		auto const b2_radius = globals::b2_scale_factor * globals::powerup_radius;

		auto shape = b2CircleShape();
		shape.m_radius = b2_radius.x;

		auto fixture_def = b2FixtureDef();
		fixture_def.shape = &shape;
		fixture_def.friction = 0.f;
		fixture_def.restitution = 0.f;
		fixture_def.filter.categoryBits = collision_category::powerup;
		fixture_def.filter.maskBits = collision_category::player;

		body->CreateFixture(&fixture_def);

		auto const entity = registry.create();

		registry.emplace<c_powerup_id>(entity, id);
		registry.emplace<c_powerup_type>(entity, type);
		registry.emplace<c_powerup_lifetime>(entity, globals::powerup_lifetime);
		registry.emplace<c_powerup_physics>(entity, body);

		body->SetUserData(reinterpret_cast<void*>(entity));

		return entity;
	}

	void destroy_powerup(entt::registry& registry, b2World& b2_world, entt::entity powerup)
	{
		auto const& pp = registry.get<c_powerup_physics>(powerup);
		b2_world.DestroyBody(pp.m_b2_body);
		registry.destroy(powerup);
	}

	// TILE:
	std::uint16_t get_tile_collision_category(tile_type type)
	{
		switch (type)
		{
		case tile_type::grass:      return collision_category::tile;
		case tile_type::road_ew:    return collision_category::tile;
		case tile_type::road_ns:    return collision_category::tile;
		case tile_type::road_cross: return collision_category::tile;
		case tile_type::building:   return collision_category::tile_wall;
		case tile_type::rubble:     return collision_category::tile;
		case tile_type::water:      return collision_category::tile_void;
		}

		bump::die();
	}

	std::uint16_t get_tile_collision_mask(tile_type type)
	{
		switch (type)
		{
		case tile_type::grass: return collision_category::none;
		case tile_type::road_ew: return collision_category::none;
		case tile_type::road_ns: return collision_category::none;
		case tile_type::road_cross: return collision_category::none;
		case tile_type::building: return collision_category::player | collision_category::bullet;
		case tile_type::rubble: return collision_category::none;
		case tile_type::water: return collision_category::player;
		}

		bump::die();
	}

	entt::entity create_tile(entt::registry& registry, b2World& b2_world, tile_type type, glm::vec2 position_px, glm::vec2 radius_px)
	{
		auto const b2_position = to_b2_vec2(globals::b2_scale_factor * position_px);

		auto body_def = b2BodyDef();
		body_def.type = b2_staticBody;
		body_def.position = b2_position;

		auto const body = b2_world.CreateBody(&body_def);

		auto const b2_radius = globals::b2_scale_factor * radius_px;

		auto shape = b2PolygonShape();
		shape.SetAsBox(b2_radius.x, b2_radius.y);
		
		auto fixture_def = b2FixtureDef();
		fixture_def.shape = &shape;
		fixture_def.friction = 0.f;
		fixture_def.restitution = 0.f;
		fixture_def.filter.categoryBits = get_tile_collision_category(type);
		fixture_def.filter.maskBits = get_tile_collision_mask(type);

		body->CreateFixture(&fixture_def);

		auto const entity = registry.create();

		registry.emplace<c_tile_type>(entity, type);
		registry.emplace<c_tile_physics>(entity, body);

		body->SetUserData(reinterpret_cast<void*>(entity));

		return entity;
	}

	// WORLD:
	void load_test_map(ta::world& world)
	{
		auto const tile_symbols = std::map<char, tile_type>
		{
			{ ' ', tile_type::grass },
			{ '-', tile_type::road_ew },
			{ '|', tile_type::road_ns },
			{ '+', tile_type::road_cross },
			{ '#', tile_type::building },
			{ 'x', tile_type::rubble },
			{ '~', tile_type::water },
		};

		auto const tile_radii = std::vector<glm::vec2>
		{
			globals::tile_radius,
			globals::tile_radius,
			globals::tile_radius,
			globals::tile_radius,
			globals::tile_radius - glm::vec2(8.f),
			globals::tile_radius,
			globals::tile_radius,
		};

		using namespace std::string_view_literals;

		auto const symbol_map = 
		{
			"   xxxxx    ###~"sv,
			"------+-----###~"sv,
			"##### |  xx ###~"sv,
			"~~~  x|  xx~~ ~~"sv,
			"~~~ ##+----+----"sv,
			" ##  x|~##~|##  "sv,
			" ##  x|~##~+--+#"sv,
			"  +---+----+#x|#"sv,
			"  |#xx|x## +--+#"sv,
			"# |###| ##x|x###"sv,
			"--+---+----+----"sv,
			" #### ~~~ ##xx  "sv,
		};

		auto map_grid = bump::grid<entt::entity, 2>({ 16, 12 });

		auto map_y = std::size_t{ 0 };

		for (auto const row : std::ranges::reverse_view(symbol_map))
		{
			auto map_x = std::size_t{ 0 };

			for (auto const symbol : row)
			{
				auto const coords = glm::size2{ map_x, map_y };

				auto const tile_type = tile_symbols.at(symbol);
				auto const radius = tile_radii.at(static_cast<std::size_t>(tile_type));
				auto const tile = create_tile(world.m_registry, world.m_b2_world, tile_type, globals::tile_radius + globals::tile_radius * 2.f * glm::vec2(coords), radius);

				map_grid.at(coords) = tile;
				++map_x;
			}

			++map_y;
		}

		world.m_tiles = std::move(map_grid);
	}
	
	void set_world_bounds(b2World& b2_world, glm::vec2 size_px)
	{
		auto const half_size = globals::b2_scale_factor * size_px * 0.5f;

		auto body_def = b2BodyDef();
		body_def.type = b2_staticBody;
		body_def.position = to_b2_vec2(half_size);

		auto const body = b2_world.CreateBody(&body_def);

		auto const boundsThickness = globals::b2_scale_factor * 10.f;
		auto const boundsFriction = 0.f;
		auto const boundsRestitution = 0.f;

		auto const make_fixture = [&] (b2PolygonShape& shape)
		{
			auto fixture_def = b2FixtureDef();
			fixture_def.shape = &shape;
			fixture_def.friction = boundsFriction;
			fixture_def.restitution = boundsRestitution;
			fixture_def.filter.categoryBits = collision_category::world_bounds;
			fixture_def.filter.maskBits = collision_category::player | collision_category::bullet;

			body->CreateFixture(&fixture_def);
		};

		// left
		{
			auto shape = b2PolygonShape{};
			shape.SetAsBox(boundsThickness, half_size.y, b2Vec2{ -half_size.x - (boundsThickness * 0.5f), 0.f }, 0.f);
			make_fixture(shape);
		}
		// right
		{
			auto shape = b2PolygonShape{};
			shape.SetAsBox(boundsThickness, half_size.y, b2Vec2{ half_size.x + (boundsThickness * 0.5f), 0.f }, 0.f);
			make_fixture(shape);
		}
		// top
		{
			auto shape = b2PolygonShape{};
			shape.SetAsBox(half_size.x, boundsThickness, b2Vec2{ 0.f, half_size.y + (boundsThickness * 0.5f) }, 0.f);
			make_fixture(shape);
		}
		// bottom
		{
			auto shape = b2PolygonShape{};
			shape.SetAsBox(half_size.x, boundsThickness, b2Vec2{ 0.f, -half_size.y - (boundsThickness * 0.5f) }, 0.f);
			make_fixture(shape);
		}
	}

	void render_world(bump::sdl::window const& window, bump::gl::renderer& renderer, ta::world& world)
	{
		// render
		{
			renderer.clear_color_buffers({ 0.39215f, 0.58431f, 0.92941f, 1.f });
			renderer.clear_depth_buffers();

			renderer.set_viewport({ 0, 0 }, glm::uvec2(window.get_size()));

			auto camera = bump::orthographic_camera();
			camera.m_projection.m_size = glm::vec2(window.get_size());
			camera.m_viewport.m_size = glm::vec2(window.get_size());
			
			auto const matrices = bump::camera_matrices(camera);

			renderer.set_blending(bump::gl::renderer::blending::BLEND);

			// render tiles
			{
				auto const tile_view = world.m_registry.view<c_tile_type>();

				for (auto y : bump::range(0, world.m_tiles.extents()[1]))
				{
					for (auto x : bump::range(0, world.m_tiles.extents()[0]))
					{
						auto const entity = world.m_tiles.at({ x, y });
						auto const& tt = tile_view.get<c_tile_type>(entity);

						auto const tile_index = static_cast<std::size_t>(tt.m_type);
						auto const tile_texture = world.m_tile_textures[tile_index];

						auto const position_px = globals::tile_radius + globals::tile_radius * 2.f * glm::vec2(x, y);

						auto model_matrix = glm::mat4(1.f);
						bump::set_position(model_matrix, glm::vec3(position_px, 0.f));

						world.m_tile_renderable.render(renderer, *tile_texture, matrices, model_matrix, globals::tile_radius * 2.f);
					}
				}
			}

			// render players
			{
				auto const player_view = world.m_registry.view<c_player_physics, c_player_movement, c_player_graphics, c_player_hp>();

				for (auto const p : player_view)
				{
					auto [pp, pm, pg, ph] = player_view.get<c_player_physics, c_player_movement, c_player_graphics, c_player_hp>(p);

					if (ph.m_hp <= 0.f)
						continue;

					auto const rotation_angle = dir_to_angle(pm.m_direction);
					auto const position_px = globals::b2_inv_scale_factor * to_glm_vec2(pp.m_b2_body->GetPosition());

					auto model_matrix = glm::mat4(1.f);
					bump::set_position(model_matrix, glm::vec3(position_px, 0.f));
					bump::set_rotation(model_matrix, glm::angleAxis(glm::radians(rotation_angle), glm::vec3(0.f, 0.f, 1.f)));

					auto& renderable = is_diagonal(pm.m_direction) ? world.m_tank_renderable_diagonal : world.m_tank_renderable;
					renderable.render(renderer, matrices, model_matrix, globals::player_radius * 2.f, pg.m_color);
				}
			}

			// render bullets
			{
				auto const player_color_view = world.m_registry.view<c_player_graphics>();
				auto const bullet_view = world.m_registry.view<c_bullet_owner_id, c_bullet_physics, c_bullet_lifetime>();

				for (auto const b : bullet_view)
				{
					auto const& [bid, bp, bl] = bullet_view.get<c_bullet_owner_id, c_bullet_physics, c_bullet_lifetime>(b);

					if (bl.m_lifetime <= 0.f)
						continue;

					auto const& pg = player_color_view.get<c_player_graphics>(bid.m_owner_id);

					auto const position_px = globals::b2_inv_scale_factor * to_glm_vec2(bp.m_b2_body->GetPosition());

					auto model_matrix = glm::mat4(1.f);
					bump::set_position(model_matrix, glm::vec3(position_px, 0.f));
					world.m_bullet_renderable.render(renderer, matrices, model_matrix, globals::bullet_radius * 2.f, pg.m_color);
				}
			}

			// render powerups
			{
				auto const powerup_view = world.m_registry.view<c_powerup_type, c_powerup_physics, c_powerup_lifetime>();
				
				for (auto const p : powerup_view)
				{
					auto const& [pt, pp, pl] = powerup_view.get<c_powerup_type, c_powerup_physics, c_powerup_lifetime>(p);

					if (pl.m_lifetime <= 0.f)
						continue;

					auto const position_px = globals::b2_inv_scale_factor * to_glm_vec2(pp.m_b2_body->GetPosition());
					auto const color = get_powerup_color(pt.m_type);

					auto model_matrix = glm::mat4(1.f);
					bump::set_position(model_matrix, glm::vec3(position_px, 0.f));
					world.m_powerup_renderable.render(renderer, matrices, model_matrix, globals::powerup_radius * 2.f, color);
				}
			}
			
			renderer.set_blending(bump::gl::renderer::blending::NONE);

			window.swap_buffers();
		}
	}

} // ta
