#include "ta_world.hpp"

#include "ta_globals.hpp"

#include <ranges>

namespace ta
{

	// PLAYER:
	entt::entity create_player(entt::registry& registry, b2World& b2_world, glm::vec2 position_px, glm::vec3 color)
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

		registry.emplace<c_player_hp>(entity, globals::player_hp);
		registry.emplace<c_player_graphics>(entity, color);
		registry.emplace<c_player_powerups>(entity);
		registry.emplace<c_player_physics>(entity, body);
		registry.emplace<c_player_movement>(entity);

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
	entt::entity create_bullet(entt::registry& registry, b2World& b2_world, entt::entity owner_id, std::int16_t owner_player_group_index, glm::vec2 position_px, glm::vec2 velocity_px)
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
	
	entt::entity create_powerup(entt::registry& registry, b2World& b2_world, powerup_type type, glm::vec2 position_px)
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

} // ta
