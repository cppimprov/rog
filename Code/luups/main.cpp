
#include "luups.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <map>
#include <cmath>
#include <filesystem>
#include <spanstream>
#include <string>
#include <string_view>

using color = glm::vec4;

struct feature
{
	std::string description;
	std::string symbol;
	color color;
};

namespace luups
{

	template<>
	struct to_lua_impl<glm::vec4>
	{
		static void to_lua(state_view lua, glm::vec4 const& value)
		{
			lua.push_new_table();

			for (auto i = 0; i != 4; ++i)
			{
				lua.push(value[i]);
				lua.set_field(-2, i + 1);
			}
		}
	};

	template<>
	struct from_lua_impl<glm::vec4>
	{
		static glm::vec4 from_lua(state_view lua, int index)
		{
			auto result = glm::vec4{ };

			for (auto i = 0; i != 4; ++i)
			{
				lua.push_field_raw(index, i + 1);
				result[i] = luups::from_lua<float>(lua, -1);
				lua.pop();
			}

			return result;
		}
	};

	template<>
	struct from_lua_impl<feature>
	{
		static feature from_lua(state_view lua, int index)
		{
			lua.push_field(index, "description");
			auto description = luups::from_lua<std::string>(lua, -1);
			lua.pop();

			lua.push_field(index, "symbol");
			auto symbol = luups::from_lua<std::string>(lua, -1);
			lua.pop();

			lua.push_field(index, "color");
			auto color = luups::from_lua<glm::vec4>(lua, -1);
			lua.pop();

			return { description, symbol, color };
		
		}
	};

} // luups

struct world_data
{
	std::map<std::string, color> colors;
	std::map<std::string, feature> features;
};

static int l_sin(lua_State* L)
{
	// auto lua = luups::state_view(L);
	// lua.push_number(std::sin(luups::check_number(lua, 1)));
	// return 1;

	auto x = luaL_checknumber(L, 1);
	lua_pushnumber(L, std::sin(x));
	return 1;
}

static int l_dir(lua_State* L)
{
	auto const path = luaL_checkstring(L, 1);

	auto ec = std::error_code{ };
	auto dir = std::filesystem::directory_iterator(path, ec);
	
	if (ec)
	{
		lua_pushnil(L);
		lua_pushstring(L, ec.message().c_str());
		return 2;
	}

	lua_newtable(L);
	int i = 1;

	for (auto const& entry : dir)
	{
		lua_pushnumber(L, i++);
		lua_pushstring(L, entry.path().string().c_str());
		lua_settable(L, -3);
	}

	return 1;
}

static const struct luaL_Reg l_dir_lib[] = {
	{ "dir", l_dir },
	{ nullptr, nullptr }
};

static int load_l_dir_lib(lua_State* L)
{
	luaL_newlib(L, l_dir_lib);
	return 1;
}

int main()
{
	using namespace luups;

	auto alloc = lua_debug_allocator();
	state lua = new_state(luups::debug_alloc, &alloc);

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
		auto color = glm::vec3{ lua.to_number(-3), lua.to_number(-2), lua.to_number(-1) };
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
		
		auto r = lua.to_number(-1);
		
		if (lua.push_field(lb_tbl, 2) != lua_type::number)
		{
			std::cerr << "Failed to get r field." << std::endl;
			return EXIT_FAILURE;
		}

		auto g = lua.to_number(-1);
		
		if (lua.push_field(lb_tbl, 3) != lua_type::number)
		{
			std::cerr << "Failed to get r field." << std::endl;
			return EXIT_FAILURE;
		}

		auto b = lua.to_number(-1);

		lua.clear();

		std::cout << "LIGHT_BROWN: " << r << ", " << g << ", " << b << std::endl;

	}
	
	{
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
	}

	{
		using namespace std::literals;

		auto t = run<std::tuple<std::string, double, int>>(lua, "t = ...; for i,v in ipairs(t) do print(i, v) end; return t", std::tuple{ "a"sv, 1.0, 2 });
		std::cout << std::get<0>(t) << ", " << std::get<1>(t) << ", " << std::get<2>(t) << std::endl;
	}

	{
		frun(lua, "data/colors.lua");
	}

	{
		if (lua.load_string(R"(
			local t = _G
			for k,v in pairs(t) do
				io.write(string.format("%s: %s\n", k, type(v)))
			end
		)") != lua_status::ok)
		{
			std::cerr << "Failed to load string: " << lua.pop_string() << std::endl;
			return EXIT_FAILURE;
		}

		auto s = std::string();

		auto const writer = [] (lua_State*, void const* data, std::size_t size, void* ud)
		{
			auto& s = *reinterpret_cast<std::string*>(ud);
			s.append(static_cast<char const*>(data), size);
			return 0;
		};

		lua.dump(writer, &s);

		lua.clear();

		struct stream_buf
		{
			std::size_t store()
			{
				is.read(buffer.data(), buffer.size());
				return static_cast<std::size_t>(is.gcount());
			}

			std::istream& is;
			std::array<char, 1024> buffer;
		};

		auto is = std::ispanstream(s);
		auto sb = stream_buf{ is };

		auto const reader = [] (lua_State*, void* data, std::size_t* size) -> char const*
		{
			auto& s = *reinterpret_cast<stream_buf*>(data);
			*size = s.store();
			return *size ? s.buffer.data() : nullptr;
		};

		if (lua.load(reader, "", &sb) != lua_status::ok)
		{
			std::cerr << "Failed to load string: " << lua.pop_string() << std::endl;
			return EXIT_FAILURE;
		}

		if (lua.call(0, 0) != lua_status::ok)
		{
			std::cerr << "Failed to call string: " << lua.pop_string() << std::endl;
			return EXIT_FAILURE;
		}
	}

	{
		lua.push_cfunction(l_sin);
		lua.set_global("testsin");

		if (lua.do_string("print('sin: ', testsin(0.5))") != lua_status::ok)
		{
			std::cerr << "Failed to call testsin: " << lua.pop_string() << std::endl;
			return EXIT_FAILURE;
		}
	}

	{
		lua.register_fn("dir", l_dir);
		run(lua, "local d = dir('.'); for i,v in ipairs(d) do print(i, v) end");
	}

	{
		lua.require("dir2", load_l_dir_lib);
		lua.pop();
		run(lua, "local d = dir2.dir('.'); for i,v in ipairs(d) do print(i, v) end");
	}

	{
		lua.new_library(l_dir_lib, 0);
		lua.set_global("dir3");
		run(lua, "local d = dir3.dir('.'); for i,v in ipairs(d) do print(i, v) end");
	}

	{
		frun(lua, "data/features.lua");

		run(lua, R"(
			local t = _G
			for k,v in pairs(t) do
				io.write(string.format("%s: %s\n", k, type(v)))
			end
		)");

		auto const features = run<std::map<std::string, feature>>(lua, "return features");
		auto const& floor = features.at("floor");
		
		std::cout << "floor: " << floor.description << ", " << floor.symbol << ", " << glm::to_string(floor.color) << std::endl;
	}

	{
		lua.push_global("colors");

		lua.push("PALE_PINK");
		lua.push(glm::vec4{ 0.95f, 0.83f, 0.84f, 1.f });
		lua.set_field(-3);

		lua.push("PALE_BLUE");
		lua.push(glm::vec4{ 0.84f, 0.93f, 0.95f, 1.f });
		lua.set_field(-3);

		lua.push_field(-1, "PALE_BLUE");
		auto const pb = lua.to<glm::vec4>();

		lua.clear();

		std::cout << "PALE_BLUE: " << glm::to_string(pb) << std::endl;
	}

	std::cout << "done!" << std::endl;
}

// todo: threads, yielding, continuations?

// todo:
	// organise code - split to separate files
	// put luaL_ related functions *outside* the state class?

// todo:
	// organize tests better...
	// put examples from lua book in namespaces, and call from main
