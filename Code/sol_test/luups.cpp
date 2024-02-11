#include "luups.hpp"

#include <array>
#include <charconv>

namespace luups
{

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

	lua_cfunction lua_state::at_panic(lua_CFunction panic_fn)
	{
		die_if(!is_open());
		return lua_atpanic(L, panic_fn);
	}

	void lua_state::error()
	{
		die_if(!is_open());
		lua_error(L);
	}

	std::string lua_state::traceback(std::string const& prefix, int level)
	{
		die_if(!is_open());
		luaL_traceback(L, L, prefix.data(), level);
		return pop_string();
	}

	std::string lua_state::print_value(int index) const
	{
		die_if(!is_open());

		auto const type = get_type(index);

		auto const to_hex_string = [&] (auto val)
		{
			auto const leading_zero_bits = std::countl_zero(val);
			auto const leading_zero_hex = leading_zero_bits / 4;

			auto chars = std::array<char, 2 + sizeof(val) * 2 + 1>();
			auto result = std::to_chars(
				chars.data() + 2 + leading_zero_hex, 
				chars.data() + 2 + (sizeof(val) * 2), val, 16);
			die_if(result.ec != std::errc());
			chars[0] = '0';
			chars[1] = 'x';
			return std::string(chars.data(), chars.size());
		};

		auto const to_pointer = [&] (int index)
		{
			return to_hex_string(reinterpret_cast<std::uintptr_t>(lua_topointer(L, index)));
		};
		
		switch (type)
		{
		case lua_type::none: return "none";
		case lua_type::nil: return "nil";
		case lua_type::boolean: return (get_boolean(index) ? "true" : "false");
		case lua_type::lightuserdata: return to_pointer(index);
		case lua_type::number: return std::to_string(get_number(index));
		case lua_type::string: return std::string(get_string(index));
		case lua_type::table: return to_pointer(index);
		case lua_type::function: return to_pointer(index);
		case lua_type::userdata: return to_pointer(index);
		case lua_type::thread: return to_pointer(index);
		}

		die();
	}

	std::string lua_state::print_stack() const
	{
		die_if(!is_open());
		
		auto result = std::string();
		result.append("stack (size " + std::to_string(size()) + "):\n");

		for (int i = 1; i <= size(); ++i)
		{
			result.append("" + std::to_string(i) + ":\t ");
			result.append(to_string(get_type(i)) + "\t" + print_value(i) + "\n");
		}

		return result;
	}

	std::string lua_state::print_globals()
	{
		die_if(!is_open());

		auto result = std::string();
		result.append("globals:\n");

		push_globals_table();
		push_nil();

		while (next_field(-2))
		{
			result.append(print_value(-2) + " -> " + print_value(-1) + "\n");
			pop();
		}

		pop();

		return result;
	}

	void lua_state::open_libraries()
	{
		die_if(!is_open());
		luaL_openlibs(L);
	}
	
	void lua_state::open_std_library(lua_std_library library)
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
	
	void lua_state::require(std::string const& module_name, lua_CFunction open_fn, bool global)
	{
		die_if(!is_open());

		// todo: check - is empty name valid?
		die_if(!open_fn);
		luaL_requiref(L, module_name.data(), open_fn, global);
		lua_pop(L, 1);
	}

	[[nodiscard]] lua_status lua_state::load_string(std::string const& code, lua_load_mode mode)
	{
		die_if(!is_open());
		auto const result = luaL_loadbufferx(L, code.data(), code.size(), code.data(), detail::get_mode_str(mode));
		return static_cast<lua_status>(result);
	}

	[[nodiscard]] lua_status lua_state::load_file(std::string const& path, lua_load_mode mode)
	{
		die_if(!is_open());
		auto const result = luaL_loadfilex(L, path.data(), detail::get_mode_str(mode));
		return static_cast<lua_status>(result);
	}

	[[nodiscard]] lua_status lua_state::call(int num_args, int num_results, int msg_handler_idx)
	{
		die_if(!is_open());
		die_if(size() < num_args + 1); // todo: is this necessary / correct?
		auto result = lua_pcall(L, num_args, num_results, msg_handler_idx);
		return static_cast<lua_status>(result);
	}

	[[nodiscard]] lua_status lua_state::do_string(std::string const& code, lua_load_mode mode, int num_results, int msg_handler_idx)
	{
		die_if(!is_open());

		auto const load_result = load_string(code, mode);

		if (load_string(code, mode) != lua_status::ok)
			return load_result;
		
		return call(0, num_results, msg_handler_idx);
	}

	[[nodiscard]] lua_status lua_state::do_file(std::string const& path, lua_load_mode mode, int num_results, int msg_handler_idx)
	{
		die_if(!is_open());

		auto const load_result = load_file(path, mode);

		if (load_result != lua_status::ok)
			return load_result;
		
		return call(0, num_results, msg_handler_idx);
	}

	int lua_state::size() const
	{
		die_if(!is_open());
		return lua_gettop(L);
	}
	
	void lua_state::resize(int index)
	{
		die_if(!is_open());
		lua_settop(L, index);
	}

	void lua_state::copy(int from_index, int to_index)
	{
		die_if(!is_open());
		lua_copy(L, from_index, to_index);
	}

	void lua_state::insert(int index)
	{
		die_if(!is_open());
		lua_insert(L, index);
	}

	void lua_state::pop(int num_elements)
	{
		die_if(!is_open());
		lua_pop(L, num_elements);
	}

	void lua_state::remove(int index)
	{
		die_if(!is_open());
		lua_remove(L, index);
	}

	void lua_state::replace(int index)
	{
		die_if(!is_open());
		lua_replace(L, index);
	}

	void lua_state::rotate(int index, int num_elements)
	{
		die_if(!is_open());
		lua_rotate(L, index, num_elements);
	}

	void lua_state::concat(int num_elements)
	{
		die_if(!is_open());
		lua_concat(L, num_elements);
	}

	bool lua_state::check(int num_elements)
	{
		die_if(!is_open());
		return (lua_checkstack(L, num_elements) != 0);
	}
	
	int lua_state::to_abs_index(int index) const
	{
		die_if(!is_open());
		// note: does not check if the index is actually valid!
		return lua_absindex(L, index);
	}

	void lua_state::push_nil()
	{
		die_if(!is_open());
		check(1);
		lua_pushnil(L);
	}

	void lua_state::push_fail()
	{
		die_if(!is_open());
		check(1);
		luaL_pushfail(L);
	}

	void lua_state::push_boolean(bool value)
	{
		die_if(!is_open());
		check(1);
		lua_pushboolean(L, value);
	}

	void lua_state::push_integer(lua_integer value)
	{
		die_if(!is_open());
		check(1);
		lua_pushinteger(L, value);
	}

	void lua_state::push_number(lua_number value)
	{
		die_if(!is_open());
		check(1);
		lua_pushnumber(L, value);
	}

	void lua_state::push_string(std::string_view value)
	{
		die_if(!is_open());
		check(1);
		lua_pushlstring(L, value.data(), value.size());
	}

	void lua_state::push_function(lua_cfunction value)
	{
		die_if(!is_open());
		check(1);
		lua_pushcfunction(L, value);
	}
	
	void lua_state::push_copy(int index)
	{
		die_if(!is_open());
		check(1);
		lua_pushvalue(L, index);
	}
	
	void lua_state::push_length(int index)
	{
		die_if(!is_open());
		check(1);
		lua_len(L, index);
	}

	bool lua_state::pop_boolean()
	{
		auto const result = get_boolean();
		pop();
		return result;
	}

	lua_integer lua_state::pop_integer()
	{
		auto const result = get_integer();
		pop();
		return result;
	}

	lua_number lua_state::pop_number()
	{
		auto const result = get_number();
		pop();
		return result;
	}

	std::string lua_state::pop_string()
	{
		auto const result = get_string();
		pop();
		return std::string(result);
	}

	lua_cfunction lua_state::pop_function()
	{
		auto const result = get_function();
		pop();
		return result;
	}

	bool lua_state::get_boolean(int index) const
	{
		die_if(!is_open());
		die_if(!is_boolean(index));
		return to_boolean_impl(index);
	}

	lua_integer lua_state::get_integer(int index) const
	{
		die_if(!is_open());
		die_if(!is_integer(index));
		return to_integer_impl(index);
	}

	lua_number lua_state::get_number(int index) const
	{
		die_if(!is_open());
		die_if(!is_number(index));
		return to_number_impl(index);
	}

	std::string_view lua_state::get_string(int index) const
	{
		die_if(!is_open());
		die_if(!is_string(index));
		return to_string_impl(index);
	}

	lua_cfunction lua_state::get_function(int index) const
	{
		die_if(!is_open());
		die_if(!is_function(index));
		return lua_tocfunction(L, index);
	}

	std::optional<bool> lua_state::try_get_boolean(int index) const
	{
		if (!is_boolean(index))
			return { };
		
		return { to_boolean_impl(index) };
	}

	std::optional<lua_integer> lua_state::try_get_integer(int index) const
	{
		if (!is_integer(index))
			return { };
		
		return { to_integer_impl(index) };
	}

	std::optional<lua_number> lua_state::try_get_number(int index) const
	{
		if (!is_number(index))
			return { };
		
		return { to_number_impl(index) };
	}

	std::optional<std::string_view> lua_state::try_get_string(int index) const
	{
		if (!is_string(index))
			return { };
		
		return { to_string_impl(index) };
	}

	std::optional<lua_cfunction> lua_state::try_get_function(int index) const
	{
		if (!is_function(index))
			return { };
		
		return { lua_tocfunction(L, index) };
	}

	bool lua_state::get_boolean_or(bool default_value, int index) const
	{
		return is_boolean(index) ? to_boolean_impl(index) : default_value;
	}

	lua_integer lua_state::get_integer_or(lua_integer default_value, int index) const
	{
		return is_integer(index) ? to_integer_impl(index) : default_value;
	}

	lua_number lua_state::get_number_or(lua_number default_value, int index) const
	{
		return is_number(index) ? to_number_impl(index) : default_value;
	}

	std::string_view lua_state::get_string_or(std::string_view default_value, int index) const
	{
		return is_string(index) ? to_string_impl(index) : default_value;
	}

	lua_cfunction lua_state::get_function_or(lua_cfunction default_value, int index) const
	{
		return is_function(index) ? lua_tocfunction(L, index) : default_value;
	}

	lua_integer lua_state::get_length(int index)
	{
		die_if(!is_open());
		return luaL_len(L, index);
	}

	lua_unsigned lua_state::get_length_raw(int index)
	{
		die_if(!is_open());
		return lua_rawlen(L, index);
	}

	void lua_state::push_new_table(int sequence_size, int hash_size)
	{
		die_if(!is_open());
		lua_createtable(L, sequence_size, hash_size);
	}

	void lua_state::push_globals_table()
	{
		die_if(!is_open());
		check(1);
		lua_pushglobaltable(L);
	}

	lua_type lua_state::push_field(int table_index)
	{
		die_if(!is_open());
		return static_cast<lua_type>(lua_gettable(L, table_index));
	}
	
	lua_type lua_state::push_field_raw(int table_index)
	{
		die_if(!is_open());
		return static_cast<lua_type>(lua_rawget(L, table_index));
	}

	lua_type lua_state::push_field(int table_index, std::string const& key_name)
	{
		die_if(!is_open());
		return static_cast<lua_type>(lua_getfield(L, table_index, key_name.data()));
	}

	lua_type lua_state::push_field_raw(int table_index, std::string const& key_name)
	{
		// note: no lua_rawgetfield...
		auto const i = to_abs_index(table_index);
		push_string(key_name);
		return push_field_raw(i);
	}

	lua_type lua_state::push_field(int table_index, std::uint64_t key_index)
	{
		die_if(!is_open());
		check(1);
		return static_cast<lua_type>(lua_geti(L, table_index, key_index));
	}

	lua_type lua_state::push_field_raw(int table_index, std::uint64_t key_index)
	{
		die_if(!is_open());
		check(1);
		return static_cast<lua_type>(lua_rawgeti(L, table_index, key_index));
	}
	
	lua_type lua_state::push_global(std::string const& name)
	{
		die_if(!is_open());
		check(1);
		return static_cast<lua_type>(lua_getglobal(L, name.data()));
	}

	void lua_state::set_field(int table_index)
	{
		die_if(!is_open());
		lua_settable(L, table_index);
	}

	void lua_state::set_field_raw(int table_index)
	{
		die_if(!is_open());
		lua_rawset(L, table_index);
	}

	void lua_state::set_field(int table_index, std::string const& key_name)
	{
		die_if(!is_open());
		lua_setfield(L, table_index, key_name.data());
	}

	void lua_state::set_field_raw(int table_index, std::string const& key_name)
	{
		// note: no lua_rawsetfield...
		auto const i = to_abs_index(table_index);
		push_string(key_name);
		insert(-2);
		set_field_raw(i);
	}

	void lua_state::set_field(int table_index, std::uint64_t key_index)
	{
		die_if(!is_open());
		lua_seti(L, table_index, key_index);
	}

	void lua_state::set_field_raw(int table_index, std::uint64_t key_index)
	{
		die_if(!is_open());
		lua_rawseti(L, table_index, key_index);
	}
	
	void lua_state::set_global(std::string const& name)
	{
		die_if(!is_open());
		lua_setglobal(L, name.data());
	}

	int lua_state::next_field(int table_index)
	{
		die_if(!is_open());
		check(1);
		return lua_next(L, table_index);
	}

	int lua_state::set_ref(int table_index)
	{
		die_if(!is_open());
		return luaL_ref(L, table_index);
	}

	void lua_state::clear_ref(int table_index, int ref)
	{
		die_if(!is_open());
		luaL_unref(L, table_index, ref);
	}

	int lua_state::default_panic_fn(lua_State* L)
	{
		auto const* msg = lua_tostring(L, -1);
		lua_pop(L, 1);
		throw std::runtime_error(msg);
	}
	
	bool lua_state::to_boolean_impl(int index) const
	{
		// n.b. assumes state is valid and type is boolean
		return lua_toboolean(L, index);
	}

	lua_integer lua_state::to_integer_impl(int index) const
	{
		// n.b. assumes state is valid and type is number *and* integer
		return lua_tointeger(L, index);
	}

	lua_number lua_state::to_number_impl(int index) const
	{
		// n.b. assumes state is valid and type is number
		return lua_tonumber(L, index);
	}

	std::string_view lua_state::to_string_impl(int index) const
	{
		// n.b. assumes state is valid and type is string
		std::size_t len = 0;
		auto str = lua_tolstring(L, index, &len);
		die_if(!str);

		return std::string_view(str, len);
	}
	
} // luups
