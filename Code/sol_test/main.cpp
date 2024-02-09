
#include <sol/sol.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>
#include <cstdlib>
#include <map>
#include <optional>

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

namespace luups
{

	inline constexpr void die_if(bool condition) { if (condition) __debugbreak(); }
	[[noreturn]] inline constexpr void die() { die_if(true); }

	enum class lua_status
	{
		ok = LUA_OK,
		runtime = LUA_ERRRUN,
		memory = LUA_ERRMEM,
		error = LUA_ERRERR,
		syntax = LUA_ERRSYNTAX,
		yield = LUA_YIELD,
		file = LUA_ERRFILE,
	};

	std::string to_string(lua_status status)
	{
		switch (status)
		{
		case lua_status::ok     : return "ok";
		case lua_status::runtime: return "runtime error";
		case lua_status::memory : return "memory allocation error";
		case lua_status::error  : return "error in error handling";
		case lua_status::syntax : return "syntax error";
		case lua_status::yield  : return "yield";
		case lua_status::file   : return "file error";
		}
		die();
	}

	enum class lua_std_library
	{
		base,
		package,
		coroutine,
		table,
		io,
		os,
		string,
		math,
		utf8,
		debug,
	};

	using lua_load_mode = std::uint32_t;
	static constexpr lua_load_mode binary = 1U << 0;
	static constexpr lua_load_mode text   = 1U << 1;

	namespace detail
	{

		inline char const* get_mode_str(lua_load_mode mode)
		{
			if (mode == (binary | text)) return "bt";
			if (mode == binary) return "b";
			if (mode == text) return "t";
			die();
		}

	} // detail

	static constexpr int lua_multiple_return = LUA_MULTRET;
	static constexpr int lua_no_msg_handler = 0;

	enum class lua_type
	{
		none = LUA_TNONE,
		nil = LUA_TNIL,
		boolean = LUA_TBOOLEAN,
		lightuserdata = LUA_TLIGHTUSERDATA,
		number = LUA_TNUMBER,
		string = LUA_TSTRING,
		table = LUA_TTABLE,
		function = LUA_TFUNCTION,
		userdata = LUA_TUSERDATA,
		thread = LUA_TTHREAD,
	};

	std::string to_string(lua_type type)
	{
		switch (type)
		{
		case lua_type::none        : return "none";
		case lua_type::nil         : return "nil";
		case lua_type::boolean     : return "boolean";
		case lua_type::lightuserdata: return "lightuserdata";
		case lua_type::number      : return "number";
		case lua_type::string      : return "string";
		case lua_type::table       : return "table";
		case lua_type::function    : return "function";
		case lua_type::userdata    : return "userdata";
		case lua_type::thread      : return "thread";
		}
		die();
	}

	class lua_state
	{
	public:

		lua_state(): L(nullptr) { }
		lua_state(lua_State* L): L(L) { }

		lua_state(lua_state const&) = delete;
		lua_state& operator=(lua_state const&) = delete;

		lua_state(lua_state&& other): L(other.L) { other.L = nullptr; }
		lua_state& operator=(lua_state&& other) { auto temp = std::move(other); std::swap(L, temp.L); return *this; }

		~lua_state() { if (L) lua_close(L); }

		bool is_open() const { return L != nullptr; }
		void close() { if (L) lua_close(L); L = nullptr; }

		lua_State* handle() const { return L; }

		lua_status status() const { return static_cast<lua_status>(::lua_status(L)); }
		double version() const { return lua_version(L); }

		void open_libraries() { die_if(!is_open()); luaL_openlibs(L); }

		void open_std_library(lua_std_library library)
		{
			die_if(!is_open());

			using lib = lua_std_library;

			switch (library)
			{
			case lib::base     : require(LUA_GNAME,       luaopen_base);      break;
			case lib::package  : require(LUA_LOADLIBNAME, luaopen_package);   break;
			case lib::coroutine: require(LUA_COLIBNAME,   luaopen_coroutine); break;
			case lib::table    : require(LUA_TABLIBNAME,  luaopen_table);     break;
			case lib::io       : require(LUA_IOLIBNAME,   luaopen_io);        break;
			case lib::os       : require(LUA_OSLIBNAME,   luaopen_os);        break;
			case lib::string   : require(LUA_STRLIBNAME,  luaopen_string);    break;
			case lib::math     : require(LUA_MATHLIBNAME, luaopen_math);      break;
			case lib::utf8     : require(LUA_UTF8LIBNAME, luaopen_utf8);      break;
			case lib::debug    : require(LUA_DBLIBNAME,   luaopen_debug);     break;
			default: die();
			}
		}

		void require(std::string const& module_name, lua_CFunction open_fn, bool global = true)
		{
			die_if(!is_open());

			// todo: check - is empty name valid?
			die_if(!open_fn);
			luaL_requiref(L, module_name.data(), open_fn, global);
			lua_pop(L, 1);
		}

		lua_type get_type(int index) const
		{
			die_if(!is_open());

			return static_cast<lua_type>(::lua_type(L, index));
		}

		// note: invalid indices (outside the currently allocated stack space) return none
		// note: indices that are valid, but currently empty return nil
		bool is_none(int index) const { return get_type(index) == lua_type::none; }
		bool is_nil(int index) const { return get_type(index) == lua_type::nil; }
		bool is_boolean(int index) const { return get_type(index) == lua_type::boolean; }
		bool is_integer(int index) const { return get_type(index) == lua_type::number && lua_isinteger(L, index); }
		bool is_number(int index)  const { return get_type(index) == lua_type::number; }
		bool is_string(int index)  const { return get_type(index) == lua_type::string; }
		bool is_table(int index)   const { return get_type(index) == lua_type::table; }

		int to_abs_index(int index) const
		{
			die_if(!is_open());
			// note: we do not check if the index is actually valid!
			return lua_absindex(L, index);
		}

		int get_size() const
		{
			die_if(!is_open());
			return lua_gettop(L);
		}

		void resize(int index)
		{
			die_if(!is_open());
			lua_settop(L, index);
		}

		void clear() { resize(0); }
		
		[[nodiscard]] bool ensure(int num_elements)
		{
			die_if(!is_open());
			die_if(num_elements < 0);
			return (lua_checkstack(L, num_elements) != 0);
		}

		void copy(int from_index, int to_index)
		{
			die_if(!is_open());
			die_if(is_none(from_index) || is_none(to_index));
			lua_copy(L, from_index, to_index);
		}

		void insert(int index)
		{
			die_if(!is_open());
			die_if(is_none(index));
			die_if(is_pseudo_index(index));
			lua_insert(L, index);
		}

		void pop(int num_elements = 1)
		{
			die_if(!is_open());
			lua_pop(L, num_elements);
		}

		void remove(int index)
		{
			die_if(!is_open());
			die_if(is_none(index));
			die_if(is_pseudo_index(index));
			lua_remove(L, index);
		}

		void replace(int index)
		{
			die_if(!is_open());
			die_if(is_none(index));
			die_if(is_pseudo_index(index));
			lua_replace(L, index);
		}

		void rotate(int index, int num_elements)
		{
			die_if(!is_open());
			die_if(is_none(index));
			die_if(is_pseudo_index(index));
			// todo: check num_elements is "not greater than the size of the slice being rotated"...
			lua_rotate(L, index, num_elements);
		}

		std::string traceback(std::string const& prefix = "", int level = 0)
		{
			die_if(!is_open());
			luaL_traceback(L, L, prefix.data(), level);
			return pop_string();
		}

private:

		bool is_pseudo_index(int index)
		{
			return index <= LUA_REGISTRYINDEX; // todo: is there a better way?
		}

		bool to_boolean_impl(int index) const
		{
			// n.b. assumes state is valid and type is boolean
			return lua_toboolean(L, index);
		}

		std::int64_t to_integer_impl(int index) const
		{
			// n.b. assumes state is valid and type is number *and* integer
			return lua_tointeger(L, index);
		}

		double to_number_impl(int index) const
		{
			// n.b. assumes state is valid and type is number
			return lua_tonumber(L, index);
		}

		std::string_view to_string_impl(int index) const
		{
			// n.b. assumes state is valid and type is string
			std::size_t len = 0;
			auto str = lua_tolstring(L, index, &len);
			die_if(!str);

			return std::string_view(str, len);
		}

public:

		bool get_boolean(int index = -1) const
		{
			die_if(!is_open());
			die_if(!is_boolean(index));
			return to_boolean_impl(index);
		}

		std::int64_t get_integer(int index = -1) const
		{
			die_if(!is_open());
			die_if(!is_integer(index));
			return to_integer_impl(index);
		}

		double get_number(int index = -1) const
		{
			die_if(!is_open());
			die_if(!is_number(index));
			return to_number_impl(index);
		}

		std::string_view get_string(int index = -1) const
		{
			die_if(!is_open());
			die_if(!is_string(index));
			return to_string_impl(index);
		}

		std::optional<bool> try_get_boolean(int index = -1) const
		{
			if (!is_boolean(index))
				return { };
			
			return { to_boolean_impl(index) };
		}

		std::optional<std::int64_t> try_get_integer(int index = -1) const
		{
			if (!is_integer(index))
				return { };
			
			return { to_integer_impl(index) };
		}

		std::optional<double> try_get_number(int index = -1) const
		{
			if (!is_number(index))
				return { };
			
			return { to_number_impl(index) };
		}

		std::optional<std::string_view> try_get_string(int index = -1) const
		{
			if (!is_string(index))
				return { };
			
			return { to_string_impl(index) };
		}

		bool get_boolean_or(bool default_value, int index = -1) const
		{
			return is_boolean(index) ? to_boolean_impl(index) : default_value;
		}

		std::int64_t get_integer_or(std::int64_t default_value, int index = -1) const
		{
			return is_integer(index) ? to_integer_impl(index) : default_value;
		}

		double get_number_or(double default_value, int index = -1) const
		{
			return is_number(index) ? to_number_impl(index) : default_value;
		}

		std::string_view get_string_or(std::string_view default_value, int index = -1) const
		{
			return is_string(index) ? to_string_impl(index) : default_value;
		}

		void push_nil()
		{
			die_if(!is_open());
			die_if(!ensure(1));
			lua_pushnil(L);
		}

		void push_fail()
		{
			die_if(!is_open());
			die_if(!ensure(1));
			luaL_pushfail(L);
		}

		void push_boolean(bool value)
		{
			die_if(!is_open());
			die_if(!ensure(1));
			lua_pushboolean(L, value);
		}

		void push_integer(std::int64_t value)
		{
			die_if(!is_open());
			die_if(!ensure(1));
			lua_pushinteger(L, value);
		}

		void push_number(double value)
		{
			die_if(!is_open());
			die_if(!ensure(1));
			lua_pushnumber(L, value);
		}

		void push_string(std::string_view value)
		{
			die_if(!is_open());
			die_if(!ensure(1));
			lua_pushlstring(L, value.data(), value.size());
		}

		bool pop_boolean()
		{
			auto const result = get_boolean();
			pop();
			return result;
		}

		std::int64_t pop_integer()
		{
			auto const result = get_integer();
			pop();
			return result;
		}

		double pop_number()
		{
			auto const result = get_number();
			pop();
			return result;
		}

		std::string pop_string()
		{
			auto const result = get_string();
			pop();
			return std::string(result);
		}

		// todo: more generic load() function (i.e. lua_Reader wrapper?)
		// todo: load_stdin()

		[[nodiscard]] lua_status load_string(std::string const& code, lua_load_mode mode = binary | text)
		{
			die_if(!is_open());

			auto const result = luaL_loadbufferx(L, code.data(), code.size(), code.data(), detail::get_mode_str(mode));

			return static_cast<lua_status>(result);
		}

		[[nodiscard]] lua_status load_file(std::string const& path, lua_load_mode mode = binary | text)
		{
			die_if(!is_open());

			auto const result = luaL_loadfilex(L, path.data(), detail::get_mode_str(mode));

			return static_cast<lua_status>(result);
		}

		[[nodiscard]] lua_status do_string(std::string const& code, lua_load_mode mode = binary | text)
		{
			die_if(!is_open());

			auto const load_result = load_string(code, mode);

			if (load_result != lua_status::ok)
				return load_result;
			
			return call(0, lua_multiple_return, lua_no_msg_handler);
		}

		[[nodiscard]] lua_status do_file(std::string const& path, lua_load_mode mode = binary | text)
		{
			die_if(!is_open());

			auto const load_result = load_file(path, mode);

			if (load_result != lua_status::ok)
				return load_result;
			
			return call(0, lua_multiple_return, lua_no_msg_handler);
		}

		[[nodiscard]] lua_status call(int num_args, int num_results = lua_multiple_return, int msg_handler_idx = lua_no_msg_handler)
		{
			die_if(!is_open());
			die_if(get_size() < num_args + 1); // todo: is this necessary / correct?

			auto result = lua_pcall(L, num_args, num_results, msg_handler_idx);

			return static_cast<lua_status>(result);
		}

	private:

		lua_State* L;
	};

	lua_state open_new_lua_state()
	{
		return lua_state(luaL_newstate());
	}

} // luups

int main()
{
	using namespace luups;

	lua_state lua = open_new_lua_state();
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

// todo: move the stack related functions to a nested struct? so we call `lua.stack.push_boolean(true)` etc.
// todo: move stuff to a luups header, and move function definitions to a .cpp file.
// todo: debug print stack?
// todo: tables (create, push, etc.)

// todo: should various lua_state functions be const?

// todo: threads?
// todo: userdata?
// todo: iteration (next...)?
// todo: raw* functions
// todo: register c functions (and setfuncs?)
// todo: upvalues?
// todo: buffers?
// todo: libraries (newlib / newlibtable)
// todo: pushref
