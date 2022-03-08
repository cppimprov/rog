#include "rog_level.hpp"

#include "rog_feature.hpp"

#include <bump_math.hpp>

#include <queue>
#include <unordered_map>
#include <vector>

namespace rog
{

	bool is_walkable(bump::grid2<feature> const& grid, glm::size2 pos)
	{
		return !(grid.at(pos).m_flags & feature::flags::NO_WALK);
	}

	bool in_bounds(glm::size2 coords, glm::size2 extents)
	{
		return (coords.x < extents.x && coords.y < extents.y);
	}

	bool in_bounds(glm::ivec2 coords, glm::ivec2 min, glm::ivec2 max)
	{
		return coords.x >= min.x && coords.x < max.x && coords.y >= min.y && coords.y < max.y;
	}

	std::vector<glm::size2> reconstruct_path(std::unordered_map<glm::ivec2, glm::ivec2> const& parents, glm::size2 src, glm::size2 dst)
	{
		auto path = std::vector<glm::size2>();

		auto current = dst;

		while (current != src)
		{
			path.push_back(current);
			current = parents.at(glm::ivec2(current));
		}

		//std::reverse(path.begin(), path.end());

		return path;
	}

	std::vector<glm::size2> find_path(bump::grid2<feature> const& grid, glm::size2 src, glm::size2 dst)
	{
		bump::die_if(src.x >= grid.extents().x);
		bump::die_if(src.y >= grid.extents().y);
		bump::die_if(dst.x >= grid.extents().x);
		bump::die_if(dst.y >= grid.extents().y);

		if (src == dst) return { };

		using cost_t = int;
		using coords_t = glm::ivec2;
		struct cost_coords { cost_t m_cost; coords_t m_coords; };
		auto constexpr frontier_order = [] (cost_coords const& a, cost_coords const& b) { return a.m_cost > b.m_cost; };

		using frontier_t = std::priority_queue<cost_coords, std::vector<cost_coords>, decltype(frontier_order)>;
		using parents_t = std::unordered_map<coords_t, coords_t>;
		using costs_t = std::unordered_map<coords_t, cost_t>;

		auto frontier = frontier_t();
		auto parents = parents_t();
		auto costs = costs_t();
		
		frontier.push({ 0, coords_t(src) });
		parents.emplace(coords_t(src), coords_t(src));
		costs.emplace(coords_t(src), 0);

		auto constexpr offsets =
		{ 
			glm::ivec2{ -1, -1 }, glm::ivec2{  0, -1 }, glm::ivec2{ +1, -1 },
			glm::ivec2{ -1,  0 },                       glm::ivec2{ +1,  0 },
			glm::ivec2{ -1, +1 }, glm::ivec2{  0, +1 }, glm::ivec2{ +1, +1 },
		};

		auto constexpr heuristic_fn = [] (coords_t const& a, coords_t const& b)
		{
			// "radial distance" metric (use diagonals to minimize distance)
			auto const d = glm::abs(a - b);
			return glm::max(d.x, d.y);
		};

		while (!frontier.empty())
		{
			auto const current = frontier.top().m_coords;
			frontier.pop();

			if (current == coords_t{ dst })
				break;
			
			for (auto offset : offsets)
			{
				auto const next = current + offset;

				if (!in_bounds(next, { 0, 0 }, coords_t(grid.extents())))
					continue;
				
				if (!is_walkable(grid, glm::size2(next)))
					continue;

				auto const cost = costs.at(current) + 1;

				if (!parents.count(next) || cost < costs.at(next))
				{
					auto const h = heuristic_fn(next, coords_t{ dst });

					frontier.push({ cost + h, next });
					parents[next] = current;
					costs[next] = cost;
				}
			}
		}

		if (!parents.count(coords_t{ dst }))
			return { }; // failed to find a path
		
		return reconstruct_path(parents, src, dst);
	}
	
} // rog
