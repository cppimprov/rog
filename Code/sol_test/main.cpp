
#include "luups.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

	lua_state lua = new_state();
	lua.open_libraries();
	
	auto result = lua.do_string("print('Hello, world!')");
	die_if(result != lua_status::ok);

	world_data data;

	if (lua.do_file("data/colors.lua") != lua_status::ok)
	{
		std::cerr << "Failed to load colors.lua: " << lua.pop_string() << std::endl;
		return EXIT_FAILURE;
	}

	try
	{
		lua.push_boolean(false);
		lua.insert(LUA_REGISTRYINDEX);
	}
	catch (std::exception const& e)
	{
		std::cerr << e.what() << std::endl;
	}

	// lua.do<glm::vec3>("return colors.LIGHT_BROWN");
	// lua.do("x, y = ...; print(x, y);", 1, 2);

	std::cout << lua.print_globals() << std::endl;

	if (lua.push_global("colors") != lua_type::table)
	{
		std::cerr << "Failed to get global colors table." << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << lua.print_stack() << std::endl;

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
	
	std::cout << "done!" << std::endl;
}

// todo: remove checks in functions that are now covered by api asserts in lua
// todo: throw instead of die in lua_assert

