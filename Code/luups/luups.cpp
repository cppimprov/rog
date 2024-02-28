#include "luups.hpp"

#include <array>
#include <charconv>

namespace luups
{

	char const* to_c_str(lua_status status)
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
	
	char const* to_c_str(lua_type type)
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

	lua_cfunction state_view::at_panic(lua_cfunction panic_fn)
	{
		return lua_atpanic(L, panic_fn);
	}

	void state_view::error()
	{
		lua_error(L);
	}

	void state_view::warning(std::string const& msg)
	{
		lua_warning(L, msg.data(), false);
	}

	int state_view::dump(lua_writer writer, void* ud, bool strip_debug_info)
	{
		return lua_dump(L, writer, ud, strip_debug_info ? 1 : 0);
	}

	std::string state_view::print_value(int index) const
	{
		auto const type = get_type(index);

		auto const to_hex_string = [&] (auto val)
		{
			auto const leading_zero_bits = std::countl_zero(val);
			auto const leading_zero_hex = leading_zero_bits / 4;

			auto chars = std::array<char, 2 + sizeof(val) * 2 + 1>{ 0 };
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
		case lua_type::boolean: return (to_boolean(index) ? "true" : "false");
		case lua_type::lightuserdata: return to_pointer(index);
		case lua_type::number: return std::to_string(to_number(index));
		case lua_type::string: return std::string(to_string(index));
		case lua_type::table: return to_pointer(index);
		case lua_type::function: return to_pointer(index);
		case lua_type::userdata: return to_pointer(index);
		case lua_type::thread: return to_pointer(index);
		}

		die();
	}

	std::string state_view::print_stack() const
	{
		auto result = std::string();
		result.append("stack (size " + std::to_string(size()) + "):\n");

		for (int i = 1; i <= size(); ++i)
		{
			result.append("" + std::to_string(i) + ":\t ");
			result.append(luups::to_string(get_type(i)) + "\t" + print_value(i) + "\n");
		}

		return result;
	}

	std::string state_view::print_globals()
	{
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
	
	std::string state_view::traceback(std::string const& prefix, int level)
	{
		luaL_traceback(L, L, prefix.data(), level);
		return pop_string();
	}

	void state_view::set_allocator(lua_allocator alloc, void* ud)
	{
		lua_setallocf(L, alloc, ud);
	}

	lua_allocator state_view::get_allocator(void** ud) const
	{
		return lua_getallocf(L, ud);
	}

	void state_view::gc_collect()
	{
		lua_gc(L, LUA_GCCOLLECT);
	}

	void state_view::gc_step(int step_size_kb)
	{
		lua_gc(L, LUA_GCSTEP, step_size_kb);
	}

	void state_view::gc_stop()
	{
		lua_gc(L, LUA_GCSTOP);
	}

	void state_view::gc_restart()
	{
		lua_gc(L, LUA_GCRESTART);
	}

	bool state_view::gc_is_running() const
	{
		return (lua_gc(L, LUA_GCISRUNNING) != 0);
	}

	lua_gc_mode state_view::gc_inc(int pause, int step_mul, int step_size)
	{
		return static_cast<lua_gc_mode>(lua_gc(L, LUA_GCINC, pause, step_mul, step_size));
	}

	lua_gc_mode state_view::gc_gen(int minor_mul, int major_mul)
	{
		return static_cast<lua_gc_mode>(lua_gc(L, LUA_GCGEN, minor_mul, major_mul));
	}

	int state_view::gc_count_bytes() const
	{
		return lua_gc(L, LUA_GCCOUNT) * 1024 + lua_gc(L, LUA_GCCOUNTB);
	}

	void state_view::open_libraries()
	{
		luaL_openlibs(L);
	}
	
	void state_view::open_std_library(lua_std_library library)
	{
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
	
	void state_view::require(std::string const& module_name, lua_cfunction open_fn, bool global)
	{
		luaL_requiref(L, module_name.data(), open_fn, global);
	}

	void state_view::register_fn(std::string const& name, lua_cfunction fn)
	{
		lua_register(L, name.data(), fn);
	}
	
	void state_view::new_library(std::span<lua_reg const> fns, int num_upvalues)
	{
		push_new_table(detail::narrow_cast<int>(fns.size()), 0);
		register_fns(fns, num_upvalues);
	}
	
	void state_view::register_fns(std::span<lua_reg const> fns, int num_upvalues)
	{
		// note: this is a re-implementation of luaL_setfuncs that doesn't require
		// the fns table to be null-terminated.

		check_stack(num_upvalues, "too many upvalues");

		for (auto const& fn : fns)
		{
			if (!fn.name)
				continue;

			if (!fn.func)
			{
				push_boolean(false);
			}
			else
			{
				for (auto i = 0; i != num_upvalues; ++i)
					push_copy(-num_upvalues);
				
				push_cclosure(fn.func, num_upvalues);
			}

			set_field(-(num_upvalues + 2), fn.name);
		}

		pop(num_upvalues);
	}

	[[nodiscard]] lua_status state_view::load(lua_reader reader, std::string const& chunk_name, void* ud)
	{
		auto const result = lua_load(L, reader, ud, chunk_name.data(), nullptr);
		return static_cast<lua_status>(result);
	}

	[[nodiscard]] lua_status state_view::load_string(std::string const& code, std::string const& chunk_name)
	{
		auto const result = luaL_loadbufferx(L, code.data(), code.size(), chunk_name.data(), detail::get_mode_str(load_mode));
		return static_cast<lua_status>(result);
	}

	[[nodiscard]] lua_status state_view::load_file(std::string const& path)
	{
		auto const result = luaL_loadfilex(L, path.data(), detail::get_mode_str(load_mode));
		return static_cast<lua_status>(result);
	}

	[[nodiscard]] lua_status state_view::load_stdin()
	{
		auto const result = luaL_loadfilex(L, nullptr, detail::get_mode_str(load_mode));
		return static_cast<lua_status>(result);
	}

	[[nodiscard]] lua_status state_view::call(int num_args, int num_results)
	{
		auto result = lua_pcall(L, num_args, num_results, msg_handler_idx);
		return static_cast<lua_status>(result);
	}

	[[nodiscard]] void state_view::call_unprotected(int num_args, int num_results)
	{
		lua_call(L, num_args, num_results);
	}

	[[nodiscard]] lua_status state_view::do_string(std::string const& code, std::string const& chunk_name)
	{
		auto const load_result = load_string(code, chunk_name);

		if (load_result != lua_status::ok)
			return load_result;

		return call(0, lua_multiple_return);
	}

	[[nodiscard]] lua_status state_view::do_file(std::string const& path)
	{
		auto const load_result = load_file(path);

		if (load_result != lua_status::ok)
			return load_result;
		
		return call(0, lua_multiple_return);
	}

	int state_view::size() const
	{
		return lua_gettop(L);
	}
	
	void state_view::resize(int index)
	{
		lua_settop(L, index);
	}

	void state_view::copy(int from_index, int to_index)
	{
		lua_copy(L, from_index, to_index);
	}

	void state_view::push_copy(int index)
	{
		check_stack(1);
		lua_pushvalue(L, index);
	}
	
	void state_view::insert(int index)
	{
		lua_insert(L, index);
	}

	void state_view::pop(int num_elements)
	{
		lua_pop(L, num_elements);
	}

	void state_view::remove(int index)
	{
		lua_remove(L, index);
	}

	void state_view::replace(int index)
	{
		lua_replace(L, index);
	}

	void state_view::rotate(int index, int num_elements)
	{
		lua_rotate(L, index, num_elements);
	}

	void state_view::concat(int num_elements)
	{
		lua_concat(L, num_elements);
	}

	void state_view::gsub(std::string const& pattern, std::string const& replacement, std::string const& subject)
	{
		luaL_gsub(L, subject.data(), pattern.data(), replacement.data());
	}

	[[nodiscard]] bool state_view::check(int num_elements)
	{
		return (lua_checkstack(L, num_elements) != 0);
	}

	void state_view::check_stack(int num_elements, std::string const& msg)
	{
		luaL_checkstack(L, num_elements, !msg.empty() ? msg.data() : nullptr);
	}
	
	int state_view::to_abs_index(int index) const
	{
		// note: does not check if the index is actually valid!
		return lua_absindex(L, index);
	}

	void state_view::check_none(int index) const
	{
		if (!is_none(index))
			luaL_typeerror(L, index, to_c_str(lua_type::none));
	}

	void state_view::check_nil(int index) const
	{
		if (!is_nil(index))
			luaL_typeerror(L, index, to_c_str(lua_type::nil));
	}

	void state_view::check_boolean(int index) const
	{
		if (!is_boolean(index))
			luaL_typeerror(L, index, to_c_str(lua_type::boolean));
	}

	void state_view::check_integer(int index) const
	{
		if (!is_integer(index))
			luaL_typeerror(L, index, "integer");
	}

	void state_view::check_number(int index) const
	{
		if (!is_number(index))
			luaL_typeerror(L, index, to_c_str(lua_type::number));
	}

	void state_view::check_string(int index) const
	{
		if (!is_string(index))
			luaL_typeerror(L, index, to_c_str(lua_type::string));
	}

	void state_view::check_table(int index) const
	{
		if (!is_table(index))
			luaL_typeerror(L, index, to_c_str(lua_type::table));
	}

	void state_view::check_function(int index) const
	{
		if (!is_function(index))
			luaL_typeerror(L, index, to_c_str(lua_type::function));
	}

	void state_view::check_cfunction(int index) const
	{
		if (!is_cfunction(index))
			luaL_typeerror(L, index, "cfunction");
	}

	void state_view::check_userdata(int index) const
	{
		if (!is_userdata(index))
			luaL_typeerror(L, index, to_c_str(lua_type::userdata));
	}

	void state_view::check_lightuserdata(int index) const
	{
		if (!is_lightuserdata(index))
			luaL_typeerror(L, index, to_c_str(lua_type::lightuserdata));
	}

	void state_view::check_thread(int index) const
	{
		if (!is_thread(index))
			luaL_typeerror(L, index, to_c_str(lua_type::thread));
	}

	void state_view::push_nil()
	{
		check_stack(1);
		lua_pushnil(L);
	}

	void state_view::push_fail()
	{
		check_stack(1);
		luaL_pushfail(L);
	}

	void state_view::push_boolean(bool value)
	{
		check_stack(1);
		lua_pushboolean(L, value);
	}

	void state_view::push_integer(lua_integer value)
	{
		check_stack(1);
		lua_pushinteger(L, value);
	}

	void state_view::push_number(lua_number value)
	{
		check_stack(1);
		lua_pushnumber(L, value);
	}

	void state_view::push_string(std::string_view value)
	{
		check_stack(1);
		lua_pushlstring(L, value.data(), value.size());
	}

	void state_view::push_cclosure(lua_cfunction value, int num_values)
	{
		check_stack(1);
		lua_pushcclosure(L, value, num_values);
	}

	void state_view::push_cfunction(lua_cfunction value)
	{
		check_stack(1);
		lua_pushcfunction(L, value);
	}

	void state_view::push_lightuserdata(void* value)
	{
		check_stack(1);
		lua_pushlightuserdata(L, value);
	}

	void state_view::push_fulluserdata(std::size_t block_size, int num_user_values)
	{
		check_stack(1);
		// note: the user should call to_userdata to get the memory block address
		(void)lua_newuserdatauv(L, block_size, num_user_values);
	}
	
	bool state_view::to_boolean(int index) const
	{
		check_boolean(index);
		return lua_toboolean(L, index);
	}

	lua_integer state_view::to_integer(int index) const
	{
		check_integer(index);
		return lua_tointeger(L, index);
	}

	lua_number state_view::to_number(int index) const
	{
		check_number(index);
		return lua_tonumber(L, index);
	}

	std::string_view state_view::to_string(int index) const
	{
		check_string(index);
		
		// n.b. assumes state is valid and type is string
		std::size_t len = 0;
		auto str = lua_tolstring(L, index, &len);
		die_if(!str);

		return std::string_view(str, len);
	}

	lua_cfunction state_view::to_cfunction(int index) const
	{
		check_cfunction(index);
		return lua_tocfunction(L, index);
	}

	void* state_view::to_userdata(int index) const
	{
		check_userdata(index);
		return lua_touserdata(L, index);
	}

	lua_integer state_view::get_length(int index)
	{
		return luaL_len(L, index);
	}

	lua_unsigned state_view::get_length_raw(int index)
	{
		return lua_rawlen(L, index);
	}

	void state_view::push_length(int index)
	{
		check_stack(1);
		lua_len(L, index);
	}

	void state_view::push_new_table(int sequence_size, int hash_size)
	{
		lua_createtable(L, sequence_size, hash_size);
	}

	void state_view::push_globals_table()
	{
		check_stack(1);
		lua_pushglobaltable(L);
	}

	lua_type state_view::push_field(int table_index)
	{
		return static_cast<lua_type>(lua_gettable(L, table_index));
	}
	
	lua_type state_view::push_field_raw(int table_index)
	{
		return static_cast<lua_type>(lua_rawget(L, table_index));
	}

	lua_type state_view::push_field(int table_index, std::string const& key_name)
	{
		return static_cast<lua_type>(lua_getfield(L, table_index, key_name.data()));
	}

	lua_type state_view::push_field_raw(int table_index, std::string const& key_name)
	{
		// note: no lua_rawgetfield...
		auto const i = to_abs_index(table_index);
		push_string(key_name);
		return push_field_raw(i);
	}

	lua_type state_view::push_field(int table_index, lua_integer key_index)
	{
		check_stack(1);
		return static_cast<lua_type>(lua_geti(L, table_index, key_index));
	}

	lua_type state_view::push_field_raw(int table_index, lua_integer key_index)
	{
		check_stack(1);
		return static_cast<lua_type>(lua_rawgeti(L, table_index, key_index));
	}

	lua_type state_view::push_field_raw(int table_index, void* p)
	{
		check_stack(1);
		return static_cast<lua_type>(lua_rawgetp(L, table_index, p));
	}
	
	lua_type state_view::push_global(std::string const& name)
	{
		check_stack(1);
		return static_cast<lua_type>(lua_getglobal(L, name.data()));
	}

	void state_view::set_field(int table_index)
	{
		lua_settable(L, table_index);
	}

	void state_view::set_field_raw(int table_index)
	{
		lua_rawset(L, table_index);
	}

	void state_view::set_field(int table_index, std::string const& key_name)
	{
		lua_setfield(L, table_index, key_name.data());
	}

	void state_view::set_field_raw(int table_index, std::string const& key_name)
	{
		// note: no lua_rawsetfield...
		auto const i = to_abs_index(table_index);
		push_string(key_name);
		insert(-2);
		set_field_raw(i);
	}

	void state_view::set_field(int table_index, lua_integer key_index)
	{
		lua_seti(L, table_index, key_index);
	}

	void state_view::set_field_raw(int table_index, lua_integer key_index)
	{
		lua_rawseti(L, table_index, key_index);
	}

	void state_view::set_field_raw(int table_index, void* p)
	{
		lua_rawsetp(L, table_index, p);
	}
	
	void state_view::set_global(std::string const& name)
	{
		lua_setglobal(L, name.data());
	}

	bool state_view::next_field(int table_index)
	{
		check_stack(1);
		return (lua_next(L, table_index) != 0);
	}

	void state_view::push_userdata_value(int userdata_index, int value_index)
	{
		lua_getiuservalue(L, userdata_index, value_index);
	}

	void state_view::set_userdata_value(int userdata_index, int value_index)
	{
		lua_setiuservalue(L, userdata_index, value_index);
	}

	int state_view::set_ref(int table_index)
	{
		return luaL_ref(L, table_index);
	}

	void state_view::clear_ref(int table_index, int ref)
	{
		luaL_unref(L, table_index, ref);
	}

	void state_view::arith(lua_arith_op op)
	{
		lua_arith(L, static_cast<int>(op));
	}

	bool state_view::compare(lua_compare_op op, int index1, int index2)
	{
		return (lua_compare(L, index1, index2, static_cast<int>(op)) != 0);
	}

	bool state_view::compare_eq(int index1, int index2)
	{
		return compare(lua_compare_op::eq, index1, index2);
	}

	bool state_view::compare_lt(int index1, int index2)
	{
		return compare(lua_compare_op::lt, index1, index2);
	}

	bool state_view::compare_le(int index1, int index2)
	{
		return compare(lua_compare_op::le, index1, index2);
	}

	bool state_view::raw_eq(int index1, int index2)
	{
		return (lua_rawequal(L, index1, index2) != 0);
	}

	int throw_runtime_error(lua_State* L)
	{
		auto const* msg = lua_tostring(L, -1);
		lua_pop(L, 1);
		throw std::runtime_error(msg);
	}
	
} // luups
