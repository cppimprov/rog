#include "rog_level_gen.hpp"

#include "rog_feature.hpp"
#include "rog_random.hpp"

#include <bump_die.hpp>
#include <bump_math.hpp>
#include <bump_range.hpp>

#include <map>

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
			"....."
			"....."
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

		level generate_level(std::int32_t depth)
		{
			bump::die_if(depth < 1);
			bump::die_if(depth > 5);

			if (depth == 1) return { depth, level_1, {} };
			if (depth == 2) return { depth, level_2, {} };
			if (depth == 3) return { depth, level_3, {} };

			bump::die(); // unreachable
		}
		
	} // level_gen
	
} // rog
