#pragma once

#include "bump_die.hpp"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

namespace bump
{

	namespace lua
	{

		using allocator_t = lua_Alloc;
		using reader_t = lua_Reader;
		using writer_t = lua_Writer;

		using number_t = lua_Number;
		using integer_t = lua_Integer;
		using unsigned_t = lua_Unsigned; 
		using cfunction_t = lua_CFunction;

		using reg_t = luaL_Reg;
		using state_t = lua_State;

		enum class gc_mode
		{
			incremental = LUA_GCINC,
			generational = LUA_GCGEN,
		};

		enum class status
		{
			ok = LUA_OK,
			runtime = LUA_ERRRUN,
			memory = LUA_ERRMEM,
			error = LUA_ERRERR,
			syntax = LUA_ERRSYNTAX,
			yield = LUA_YIELD,
			file = LUA_ERRFILE,
		};
		
		inline char const* to_c_str(status status)
		{
			switch (status)
			{
			case status::ok     : return "ok";
			case status::runtime: return "runtime error";
			case status::memory : return "memory allocation error";
			case status::error  : return "error in error handling";
			case status::syntax : return "syntax error";
			case status::yield  : return "yield";
			case status::file   : return "file error";
			}
			die();
		}

		inline std::string to_string(status status) { return std::string(to_c_str(status)); }

		enum class type
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
		
		inline char const* to_c_str(type type)
		{
			switch (type)
			{
			case type::none        : return "none";
			case type::nil         : return "nil";
			case type::boolean     : return "boolean";
			case type::lightuserdata: return "lightuserdata";
			case type::number      : return "number";
			case type::string      : return "string";
			case type::table       : return "table";
			case type::function    : return "function";
			case type::userdata    : return "userdata";
			case type::thread      : return "thread";
			}
			die();
		}

		inline std::string to_string(type type) { return std::string(to_c_str(type)); }

		enum class arith_op
		{
			add = LUA_OPADD,
			sub = LUA_OPSUB,
			mul = LUA_OPMUL,
			mod = LUA_OPMOD,
			pow = LUA_OPPOW,
			div = LUA_OPDIV,
			idiv = LUA_OPIDIV,
			band = LUA_OPBAND,
			bor = LUA_OPBOR,
			bxor = LUA_OPBXOR,
			shl = LUA_OPSHL,
			shr = LUA_OPSHR,
			unm = LUA_OPUNM,
			bnot = LUA_OPBNOT,
		};

		enum class compare_op
		{
			eq = LUA_OPEQ,
			lt = LUA_OPLT,
			le = LUA_OPLE,
		};

		using load_mode = std::uint32_t;
		static constexpr load_mode binary = 1U << 0;
		static constexpr load_mode text   = 1U << 1;

		namespace detail
		{

			inline char const* get_mode_str(load_mode mode)
			{
				if (mode == (binary | text)) return "bt";
				if (mode == binary) return "b";
				if (mode == text) return "t";
				bump::die();
			}

		} // detail
		
		static constexpr int multiple_return = LUA_MULTRET;
		static constexpr int no_msg_handler = 0;

		enum class std_library
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

	} // lua

} // bump
