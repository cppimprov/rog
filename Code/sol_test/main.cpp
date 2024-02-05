
#include <sol/sol.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>
#include <cstdlib>
#include <map>

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

int main() {

	sol::state lua;

	lua.open_libraries
	(
		sol::lib::base,
		sol::lib::package,
		sol::lib::coroutine,
		sol::lib::string,
		sol::lib::os,
		sol::lib::math,
		sol::lib::table,
		sol::lib::debug,
		sol::lib::bit32,
		sol::lib::io,
		sol::lib::utf8
	);

	world_data data;

	auto pfr = sol::protected_function_result();
	pfr = lua.script_file("data/colors.lua");
	pfr = lua.script_file("data/features.lua");
	
	if (!pfr.valid())
	{
		sol::error err = pfr;
		std::cout << "Failed to load colors.lua: " << err.what() << std::endl;
		return EXIT_FAILURE;
	}

	auto c = lua["colors"]["DARK_BROWN"].get<sol::as_table_t<glm::vec4>>();

	std::cout << c.x << std::endl;
	std::cout << c.y << std::endl;
	std::cout << c.z << std::endl;
	std::cout << c.w << std::endl;
	
	std::cout << "done!" << std::endl;
}
