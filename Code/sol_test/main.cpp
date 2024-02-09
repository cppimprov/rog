
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

	lua.push_boolean(true);
	lua.push_integer(42);
	
	// ...
	
	// world_data data;

	// auto lr = lua.load_file("data/colors.lua");

	// if (!lr.valid())
	// {
	// 	auto const get_status_string = [] (sol::load_status s)
	// 	{
	// 		switch (s)
	// 		{
	// 			case sol::load_status::ok: return "ok";
	// 			case sol::load_status::syntax: return "syntax error";
	// 			case sol::load_status::memory: return "memory allocation error";
	// 			case sol::load_status::gc: return "garbage collection error";
	// 			case sol::load_status::file: return "file error";
	// 		}
	// 		return "unknown";
	// 	};

	// 	std::cout << "Failed to load file nope.lua: " << get_status_string(lr.status()) << std::endl;
	// }

	// try
	// {
	// 	lua["sdoifuosidf"]["sdkljfsf"] = 5;
	// }
	// catch (std::exception& e)
	// {
	// 	std::cout << "Lua error: " << e.what() << std::endl;
	// }

	// auto env = sol::environment(lua, sol::create, lua.globals());

	// {
	// 	auto pfr = lua.script_file("data/colors.lua", env);
		
	// 	if (!pfr.valid())
	// 	{
	// 		sol::error err = pfr;
	// 		std::cout << "Failed to load colors.lua: " << err.what() << std::endl;
	// 		return EXIT_FAILURE;
	// 	}

	// 	auto c = lua["colors"]["LIGHT_BROWN"].get<sol::as_table_t<glm::vec4>>();

	// 	std::cout << c.x << std::endl;
	// 	std::cout << c.y << std::endl;
	// 	std::cout << c.z << std::endl;
	// 	std::cout << c.w << std::endl;
	// }

	// {
	// 	auto pfr = lua.script_file("data/features.lua", env);

	// 	// ...
	// }
	
	std::cout << "done!" << std::endl;
}

// todo: debug print stack?

// todo: should various lua_state functions be const?

// todo: metatables?
// todo: threads?
// todo: userdata?
// todo: iteration (next...)?
// todo: raw* functions
// todo: register c functions (and setfuncs?)
// todo: upvalues?
// todo: buffers?
// todo: libraries (newlib / newlibtable)
// todo: pushref
