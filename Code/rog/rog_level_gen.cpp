#include "rog_level_gen.hpp"

#include "rog_ecs.hpp"
#include "rog_feature.hpp"
#include "rog_monster.hpp"
#include "rog_player.hpp"
#include "rog_random.hpp"

#include <bump_die.hpp>
#include <bump_log.hpp>
#include <bump_math.hpp>
#include <bump_range.hpp>

#include <map>
#include <optional>

namespace rog
{
	
	namespace level_gen
	{

		bump::grid2<feature> level_from_string(glm::size2 size, std::string const& in)
		{
			bump::die_if(in.size() != size.x * size.y);
			
			auto const key = std::map<char, feature>
			{
				{ '.', features::floor },
				{ '#', features::wall },
				{ '<', features::stairs_up },
				{ '>', features::stairs_down },
			};

			auto grid = bump::grid2<feature>(size, features::floor);

			for (auto const y : bump::range(0, size.y))
			{
				for (auto const x : bump::range(0, size.x))
				{
					auto const index = y * size.x + x;
					auto const f = key.find(in[index]);

					bump::die_if(f == key.end());

					grid.at({ x, y }) = f->second;
				}
			}

			return grid;
		}

		auto const level_1 = level_from_string
		(
			{ 5, 4 },
			"....."
			".###."
			".###."
			"....<"
		);
		
		auto const level_2 = level_from_string
		(
			{ 5, 4 },
			">...#"
			"....#"
			"....#"
			"####<"
		);
		
		auto const level_3 = level_from_string
		(
			{ 5, 4 },
			">.###"
			"....#"
			"#.###"
			"#####"
		);

		bool place_player(level& level)
		{
			auto const level_size = level.m_grid.extents();

			auto const pos = [&] ()
			{
				// find the top-leftmost empty square
				for (auto y : bump::range(0, level_size.y))
					for (auto x : bump::range(0, level_size.x))
						if (is_walkable(level, { x, y }) && !is_occupied(level, { x, y }))
							return std::optional<glm::size2>({ x, y });

				return std::optional<glm::size2>();
			}();

			if (!pos)
				return false;

			level.m_registry.get<c_position>(level.m_player).m_pos = pos.value();

			level.m_actors.at(pos.value()) = level.m_player;

			return true;
		}

		bool place_monster(level& level, entt::handle monster_handle, random::rng_t& rng)
		{
			auto const level_size = level.size();
			bump::die_if(level_size == glm::ivec2(0));

			auto const pos = [&] ()
			{
				// find a random empty square
				auto constexpr MAX_TRIES = 1000;
				for (auto t = 0; t != MAX_TRIES; ++t)
				{
					auto const pos = random::rand_range(rng, glm::ivec2(0), level_size - glm::ivec2(1));

					if (is_walkable(level, pos) && !is_occupied(level, pos))
						return std::optional<glm::ivec2>(pos);
				}

				return std::optional<glm::ivec2>();
			}();

			if (!pos)
				return false;

			monster_handle.get<c_position>().m_pos = pos.value();

			level.m_actors.at(glm::size2(pos.value())) = monster_handle.entity();

			return true;
		}

		level generate_level(std::int32_t depth, random::rng_t& rng)
		{
			bump::die_if(depth < 1);
			bump::die_if(depth > 3);

			auto const& map = (depth == 1 ? level_1 : depth == 2 ? level_2 : level_3);

			auto level = rog::level
			{ 
				.m_depth = depth,
				.m_grid = map,
				.m_registry = {},
				.m_player = entt::null,
				.m_actors = bump::grid2<entt::entity>(map.extents(), entt::null),
			};

			// add player
			level.m_player = player_create_entity(level.m_registry); // todo: do this above (put registry before player)
			if (!place_player(level))
			{
				bump::log_info("Failed to place player!");
				bump::die();
			}
			
			auto monster = monster_create_entity(level.m_registry);
			if (!place_monster(level, { level.m_registry, monster }, rng))
			{
				bump::log_info("Failed to place monster!");
				bump::die();
			}

			return level;
		}
		
	} // level_gen
	
} // rog
