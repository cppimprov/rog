
#include "luups.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <map>
#include <string>

using color = glm::vec4;

struct feature
{
	std::string description;
	std::string symbol;
	color color;
};

struct world_data
{
	std::map<std::string, color> colors;
	std::map<std::string, feature> features;
};

int main()
{
	using namespace luups;

	auto alloc = lua_debug_allocator();

	lua_state lua = new_state(alloc);

	std::cout << "bytes allocated: " << alloc.allocated << std::endl;

	lua.open_libraries();
	
	std::cout << "bytes allocated: " << alloc.allocated << std::endl;
	
	auto result = lua.do_string("print('Hello, world!')");
	die_if(result != lua_status::ok);
	
	std::cout << "bytes allocated: " << alloc.allocated << std::endl;

	world_data data;

	if (lua.do_file("data/colors.lua") != lua_status::ok)
	{
		std::cerr << "Failed to load colors.lua: " << lua.pop_string() << std::endl;
		return EXIT_FAILURE;
	}
	
	std::cout << "bytes allocated: " << alloc.allocated << std::endl;

	lua.gc_collect();

	std::cout << "bytes allocated: " << alloc.allocated << std::endl;
	std::cout << "bytes allocated: " << lua.gc_count_bytes() << std::endl;

	try
	{
		(void)lua.do_string("return table.unpack(colors.LIGHT_BROWN)");
		auto color = glm::vec3{ lua.get_number(-3), lua.get_number(-2), lua.get_number(-1) };
		std::cout << glm::to_string(color) << std::endl;
		lua.clear();
	}
	catch (std::exception const& e)
	{
		std::cerr << e.what() << std::endl;
	}
	
	std::cout << "bytes allocated: " << alloc.allocated << std::endl;

	// lua.do<glm::vec3>("return colors.LIGHT_BROWN");
	// lua.do("x, y = ...; print(x, y);", 1, 2);

	std::cout << lua.print_globals() << std::endl;

	if (lua.push_global("colors") != lua_type::table)
	{
		std::cerr << "Failed to get global colors table." << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << lua.print_stack() << std::endl;

	{

		int c_tbl = lua.size();

		if (lua.push_field(c_tbl, "LIGHT_BROWN") != lua_type::table)
		{
			std::cerr << "Failed to get LIGHT_BROWN table." << std::endl;
			return EXIT_FAILURE;
		}

		int lb_tbl = lua.size();

		if (lua.push_field(lb_tbl, 1) != lua_type::number)
		{
			std::cerr << "Failed to get r field." << std::endl;
			return EXIT_FAILURE;
		}
		
		auto r = lua.get_number(-1);
		
		if (lua.push_field(lb_tbl, 2) != lua_type::number)
		{
			std::cerr << "Failed to get r field." << std::endl;
			return EXIT_FAILURE;
		}

		auto g = lua.get_number(-1);
		
		if (lua.push_field(lb_tbl, 3) != lua_type::number)
		{
			std::cerr << "Failed to get r field." << std::endl;
			return EXIT_FAILURE;
		}

		auto b = lua.get_number(-1);

		lua.clear();

		std::cout << "LIGHT_BROWN: " << r << ", " << g << ", " << b << std::endl;

	}

	{
		auto [rt, gt, bt] = std::tuple<int, float, int>{ 1, 2.0f, 3 };
		std::cout << gt << std::endl;
	}
	
	run(lua, "print('r1')");
	auto r2 = run<int>(lua, "return 2");
	auto [a, b, c] = run<std::int16_t, float, std::uint8_t>(lua, "return ...", 1, 2, 3 );
	auto r4 = run<double>(lua, "return 4.0");
	auto [d, e] = run<std::string, std::uint64_t>(lua, "return 'hi', 5");

	static_assert(std::is_same_v<decltype(a), std::int16_t>);
	static_assert(std::is_same_v<decltype(b), float>);
	static_assert(std::is_same_v<decltype(c), std::uint8_t>);
	static_assert(std::is_same_v<decltype(d), std::string>);
	static_assert(std::is_same_v<decltype(e), std::uint64_t>);

	std::cout << "r2: " << r2 << std::endl;
	std::cout << "r3: " << a << ", " << b << ", " << (int)c << std::endl;
	std::cout << "r4: " << r4 << std::endl;
	std::cout << "r5: " << d << ", " << e << std::endl;

	frun(lua, "data/colors.lua");

	std::cout << "done!" << std::endl;
}

// todo: way to return / send functions / function pointers to run()?

// todo: organise code - split to separate files
// todo: do all the functions in lua_state need to be members?
