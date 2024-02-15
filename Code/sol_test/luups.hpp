#pragma once

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <array>
#include <concepts>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace luups
{

	inline constexpr void die_if(bool condition) { if (condition) __debugbreak(); }
	[[noreturn]] inline constexpr void die() { die_if(true); }

	using lua_allocator = lua_Alloc;

	class lua_debug_allocator
	{
	public:

		lua_debug_allocator() = default;

		lua_debug_allocator(lua_debug_allocator const&) = delete;
		lua_debug_allocator& operator=(lua_debug_allocator const&) = delete;
		lua_debug_allocator(lua_debug_allocator&&) = delete;
		lua_debug_allocator& operator=(lua_debug_allocator&&) = delete;

		void *realloc(void* ptr, std::size_t old_size, std::size_t new_size)
		{
			allocated -= old_size;
			allocated += new_size;

			if (new_size == 0)
			{
				std::free(ptr);
				return nullptr;
			}

			return std::realloc(ptr, new_size);
		}

		// todo: tag allocations with a string for debugging purposes

		std::size_t allocated = 0;
	};
	
	inline void* debug_alloc(void* ud, void* ptr, std::size_t old_size, std::size_t new_size)
	{
		return static_cast<lua_debug_allocator*>(ud)->realloc(ptr, old_size, new_size);
	}

	enum class lua_gc_mode
	{
		incremental = LUA_GCINC,
		generational = LUA_GCGEN,
	};

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

	std::string to_string(lua_status status);

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

	std::string to_string(lua_type type);

	using lua_number = lua_Number;
	using lua_integer = lua_Integer;
	using lua_unsigned = lua_Unsigned;
	using lua_cfunction = lua_CFunction;

	class lua_state
	{
	public:

		// CONSTRUCT / DESTROY

		lua_state(): L(nullptr) { }
		explicit lua_state(lua_State* L): L(L) { if (is_open()) at_panic(&default_panic_fn); }

		lua_state(lua_state const&) = delete;
		lua_state& operator=(lua_state const&) = delete;
		lua_state(lua_state&& other): L(other.L) { other.L = nullptr; }
		lua_state& operator=(lua_state&& other) { auto temp = std::move(other); std::swap(L, temp.L); return *this; }

		~lua_state() { close(); }

		bool is_open() const { return L != nullptr; }
		void close() { if (is_open()) lua_close(L); L = nullptr; }
		
		lua_State* handle() const { return L; }

		// UTILS

		lua_cfunction at_panic(lua_cfunction panic_fn);
		void error();
		void warning(std::string const& msg);

		lua_number version() const { die_if(!is_open()); return lua_version(L); }
		lua_status status() const { die_if(!is_open()); return static_cast<lua_status>(::lua_status(L)); }

		std::string traceback(std::string const& prefix = "", int level = 0);

		std::string print_value(int index) const;
		std::string print_stack() const;
		std::string print_globals();

		lua_load_mode set_load_mode(lua_load_mode mode) { auto old_mode = load_mode; load_mode = mode; return old_mode; }

		// MEMORY

		void set_allocator(lua_allocator alloc, void* ud);
		lua_allocator get_allocator(void** ud) const;

		void gc_collect();
		void gc_step(int step_size_kb);
		void gc_stop();
		void gc_restart();
		bool gc_is_running() const;
		lua_gc_mode gc_inc(int pause, int step_mul, int step_size);
		lua_gc_mode gc_gen(int minor_mul, int major_mul);
		int gc_count_bytes() const;

		// LIBRARIES / PACKAGES

		void open_libraries();
		void open_std_library(lua_std_library library);
		void require(std::string const& module_name, lua_CFunction open_fn, bool global = true);

		// LOAD / EXECUTE
		
		// todo: more generic load() function (i.e. lua_Reader wrapper?)
		// todo: load_stdin()
		// todo: improve function names?

		[[nodiscard]] lua_status load_string(std::string const& code);
		[[nodiscard]] lua_status load_file(std::string const& path);
		[[nodiscard]] lua_status call(int num_args, int num_results = lua_multiple_return, int msg_handler_idx = lua_no_msg_handler);
		[[nodiscard]] lua_status do_string(std::string const& code, int num_results = lua_multiple_return, int msg_handler_idx = lua_no_msg_handler);
		[[nodiscard]] lua_status do_file(std::string const& path, int num_results = lua_multiple_return, int msg_handler_idx = lua_no_msg_handler);

		// STACK - MANAGEMENT

		int size() const;
		bool empty() const { return size() == 0; }
		void resize(int index);
		void clear() { resize(0); }

		void copy(int from_index, int to_index);
		void insert(int index);
		void pop(int num_elements = 1);
		void remove(int index);
		void replace(int index);
		void rotate(int index, int num_elements);
		void concat(int num_elements);
		
		bool check(int num_elements);
		int to_abs_index(int index) const;

		// STACK - TYPES

		// note: invalid indices (outside the currently allocated stack space) return none
		// note: indices that are valid, but currently empty return nil
		lua_type get_type(int index) const { die_if(!is_open()); return static_cast<lua_type>(::lua_type(L, index)); }

		bool is_none(int index) const { return get_type(index) == lua_type::none; }
		bool is_nil(int index) const { return get_type(index) == lua_type::nil; }
		bool is_none_or_nil(int index) const { return is_none(index) || is_nil(index); }
		bool is_boolean(int index) const { return get_type(index) == lua_type::boolean; }
		bool is_integer(int index) const { return get_type(index) == lua_type::number && lua_isinteger(L, index); }
		bool is_number(int index) const { return get_type(index) == lua_type::number; }
		bool is_string(int index) const { return get_type(index) == lua_type::string; }
		bool is_table(int index) const { return get_type(index) == lua_type::table; }
		bool is_function(int index) const { return get_type(index) == lua_type::function; }
		bool is_cfunction(int index) const { return get_type(index) == lua_type::function && lua_iscfunction(L, index); }
		bool is_userdata(int index) const { return get_type(index) == lua_type::userdata; }
		bool is_lightuserdata(int index) const { return get_type(index) == lua_type::lightuserdata; }

		// STACK - VARIABLES

		void push_nil();
		void push_fail();
		void push_boolean(bool value);
		void push_integer(lua_integer value);
		void push_number(lua_number value);
		void push_string(std::string_view value);
		void push_cclosure(lua_cfunction value, int num_values);
		void push_cfunction(lua_cfunction value);

		void push_copy(int index);
		void push_length(int index);

		bool pop_boolean();
		lua_integer pop_integer();
		lua_number pop_number();
		std::string pop_string();
		lua_cfunction pop_cfunction();

		bool get_boolean(int index = -1) const;
		lua_integer get_integer(int index = -1) const;
		lua_number get_number(int index = -1) const;
		std::string_view get_string(int index = -1) const;
		lua_cfunction get_cfunction(int index = -1) const;

		std::optional<bool> try_get_boolean(int index = -1) const;
		std::optional<lua_integer> try_get_integer(int index = -1) const;
		std::optional<lua_number> try_get_number(int index = -1) const;
		std::optional<std::string_view> try_get_string(int index = -1) const;
		std::optional<lua_cfunction> try_get_cfunction(int index = -1) const;

		bool get_boolean_or(bool default_value, int index = -1) const;
		lua_integer get_integer_or(lua_integer default_value, int index = -1) const;
		lua_number get_number_or(lua_number default_value, int index = -1) const;
		std::string_view get_string_or(std::string_view default_value, int index = -1) const;
		lua_cfunction get_cfunction_or(lua_cfunction default_value, int index = -1) const;

		lua_integer get_length(int index);
		lua_unsigned get_length_raw(int index);

		// STACK - TABLES

		void push_new_table(int sequence_size = 0, int hash_size = 0);
		void push_globals_table();

		lua_type push_field(int table_index);
		lua_type push_field_raw(int table_index);
		lua_type push_field(int table_index, std::string const& key_name);
		lua_type push_field_raw(int table_index, std::string const& key_name);
		lua_type push_field(int table_index, lua_integer key_index);
		lua_type push_field_raw(int table_index, lua_integer key_index);
		lua_type push_global(std::string const& key_name);

		void set_field(int table_index);
		void set_field_raw(int table_index);
		void set_field(int table_index, std::string const& key_name);
		void set_field_raw(int table_index, std::string const& key_name);
		void set_field(int table_index, lua_integer key_index);
		void set_field_raw(int table_index, lua_integer key_index);
		void set_global(std::string const& key_name);

		bool next_field(int table_index);

		// STACK - REFERENCES

		int set_ref(int table_index);
		void clear_ref(int table_index, int ref);

	private:

		// THE STATE

		lua_State* L;

		// ERROR HANDLING

		static int default_panic_fn(lua_State* L);

		// LOADING

		lua_load_mode load_mode = binary | text;

		// STACK MANAGEMENT UTILS

		bool to_boolean_impl(int index) const;
		lua_integer to_integer_impl(int index) const;
		lua_number to_number_impl(int index) const;
		std::string_view to_string_impl(int index) const;
		lua_cfunction to_cfunction_impl(int index) const;
	};
	

	template<class T> struct to_lua_impl;

	template<class T>
	void to_lua(lua_state& lua, T const& value)
	{
		return to_lua_impl<std::decay_t<T>>::to_lua(lua, value);
	}

	template<class T> struct from_lua_impl;

	template<class T>
	T from_lua(lua_state& lua, int index)
	{
		return from_lua_impl<std::decay_t<T>>::from_lua(lua, index);
	}
	
	template<>
	struct to_lua_impl<bool>
	{
		static void to_lua(lua_state& lua, bool value) { lua.push_boolean(value); }
	};

	template<>
	struct from_lua_impl<bool>
	{
		static bool from_lua(lua_state& lua, int index) { return lua.get_boolean(index); }
	};

	namespace detail
	{

		template<class T, class U> requires std::is_integral_v<T> && std::is_integral_v<U>
		T narrow_cast(U u)
		{
			if constexpr (std::is_same_v<T, U>)
			{
				return u;
			}
			else
			{
				auto const t = static_cast<T>(u);

				if (static_cast<U>(t) != u)
					throw std::runtime_error("narrow_cast: value out of range");

				if constexpr (std::is_signed_v<T> != std::is_signed_v<U>)
					if ((t < T{ 0 }) != (u < U{ 0 }))
						throw std::runtime_error("narrow_cast: value out of range");

				return t;
			}
		}

		template<class T, class U> requires std::is_floating_point_v<T> && std::is_floating_point_v<U>
		T narrow_cast(U u)
		{
			if constexpr (std::is_same_v<T, U>)
			{
				return u;
			}
			else
			{
				if (u > std::numeric_limits<T>::max() || u < std::numeric_limits<T>::lowest())
					throw std::runtime_error("narrow_cast: value out of range");
				
				return static_cast<T>(u);
			}
		}

	} // detail

	// INTEGER TYPES

	template<> struct to_lua_impl<char>               { static void to_lua(lua_state& lua, char value) {               lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<signed char>        { static void to_lua(lua_state& lua, signed char value) {        lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<unsigned char>      { static void to_lua(lua_state& lua, unsigned char value) {      lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<short>              { static void to_lua(lua_state& lua, short value) {              lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<unsigned short>     { static void to_lua(lua_state& lua, unsigned short value) {     lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<int>                { static void to_lua(lua_state& lua, int value) {                lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<unsigned int>       { static void to_lua(lua_state& lua, unsigned int value) {       lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<long>               { static void to_lua(lua_state& lua, long value) {               lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<unsigned long>      { static void to_lua(lua_state& lua, unsigned long value) {      lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<long long>          { static void to_lua(lua_state& lua, long long value) {          lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<unsigned long long> { static void to_lua(lua_state& lua, unsigned long long value) { lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<char8_t>            { static void to_lua(lua_state& lua, char8_t value) {            lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<char16_t>           { static void to_lua(lua_state& lua, char16_t value) {           lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<char32_t>           { static void to_lua(lua_state& lua, char32_t value) {           lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };
	template<> struct to_lua_impl<wchar_t>            { static void to_lua(lua_state& lua, wchar_t value) {            lua.push_integer(detail::narrow_cast<lua_integer>(value)); } };

	template<> struct from_lua_impl<char>               { static char from_lua(lua_state& lua, int index) {               return detail::narrow_cast<char>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<signed char>        { static signed char from_lua(lua_state& lua, int index) {        return detail::narrow_cast<signed char>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<unsigned char>      { static unsigned char from_lua(lua_state& lua, int index) {      return detail::narrow_cast<unsigned char>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<short>              { static short from_lua(lua_state& lua, int index) {              return detail::narrow_cast<short>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<unsigned short>     { static unsigned short from_lua(lua_state& lua, int index) {     return detail::narrow_cast<unsigned short>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<int>                { static int from_lua(lua_state& lua, int index) {                return detail::narrow_cast<int>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<unsigned int>       { static unsigned int from_lua(lua_state& lua, int index) {       return detail::narrow_cast<unsigned int>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<long>               { static long from_lua(lua_state& lua, int index) {               return detail::narrow_cast<long>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<unsigned long>      { static unsigned long from_lua(lua_state& lua, int index) {      return detail::narrow_cast<unsigned long>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<long long>          { static long long from_lua(lua_state& lua, int index) {          return detail::narrow_cast<long long>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<unsigned long long> { static unsigned long long from_lua(lua_state& lua, int index) { return detail::narrow_cast<unsigned long long>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<char8_t>            { static char8_t from_lua(lua_state& lua, int index) {            return detail::narrow_cast<char8_t>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<char16_t>           { static char16_t from_lua(lua_state& lua, int index) {           return detail::narrow_cast<char16_t>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<char32_t>           { static char32_t from_lua(lua_state& lua, int index) {           return detail::narrow_cast<char32_t>(lua.get_integer(index)); } };
	template<> struct from_lua_impl<wchar_t>            { static wchar_t from_lua(lua_state& lua, int index) {            return detail::narrow_cast<wchar_t>(lua.get_integer(index)); } };

	// FLOATING POINT TYPES

	template<> struct to_lua_impl<float>       { static void to_lua(lua_state& lua, float value) {       lua.push_number(detail::narrow_cast<lua_number>(value)); } };
	template<> struct to_lua_impl<double>      { static void to_lua(lua_state& lua, double value) {      lua.push_number(detail::narrow_cast<lua_number>(value)); } };
	template<> struct to_lua_impl<long double> { static void to_lua(lua_state& lua, long double value) { lua.push_number(detail::narrow_cast<lua_number>(value)); } };

	template<> struct from_lua_impl<float>       { static float from_lua(lua_state& lua, int index) {       return detail::narrow_cast<float>(lua.get_number(index)); } };
	template<> struct from_lua_impl<double>      { static double from_lua(lua_state& lua, int index) {      return detail::narrow_cast<double>(lua.get_number(index)); } };
	template<> struct from_lua_impl<long double> { static long double from_lua(lua_state& lua, int index) { return detail::narrow_cast<long double>(lua.get_number(index)); } };

	// STRINGS

	template<>
	struct to_lua_impl<std::string>
	{
		static void to_lua(lua_state& lua, std::string const& value) { lua.push_string(value); }
	};

	template<>
	struct to_lua_impl<std::string_view>
	{
		static void to_lua(lua_state& lua, std::string_view value) { lua.push_string(value); }
	};

	template<>
	struct from_lua_impl<std::string>
	{
		static std::string from_lua(lua_state& lua, int index) { return std::string(lua.get_string(index)); }
	};

	// ARRAY

	template<class T, std::size_t N>
	struct to_lua_impl<std::array<T, N>>
	{
		static void to_lua(lua_state& lua, std::array<T, N> const& arr)
		{
			lua.push_new_table(detail::narrow_cast<int>(N));

			for (auto i = std::size_t{ 0 }; i != N; ++i)
			{
				luups::to_lua(lua, arr[i]);
				lua.set_field(-2, detail::narrow_cast<lua_integer>(i + 1));
			}
		}
	};

	template<class T, std::size_t N>
	struct from_lua_impl<std::array<T, N>>
	{

		// note: iterates indices covering the entire array so holes (nil values)
		// are ignored, and left as default constructed values.
		// if values are present, they are expected to be the correct type (for now).

		static std::array<T, N> from_lua(lua_state& lua, int index)
		{
			auto arr = std::array<T, N>{ };

			for (auto i = std::size_t{ 0 }; i != N; ++i)
			{
				lua.push_field_raw(index, detail::narrow_cast<lua_integer>(i + 1));

				if (lua.is_nil(-1))
				{
					lua.pop();
					continue;
				}

				arr[i] = luups::from_lua<T>(lua);
				lua.pop();
			}

			return arr;
		}
	};

	// VECTOR

	template<class T>
	struct to_lua_impl<std::vector<T>>
	{
		static void to_lua(lua_state& lua, std::vector<T> const& vec)
		{
			lua.push_new_table(detail::narrow_cast<int>(vec.size()));

			for (auto i = std::size_t{ 0 }; i != vec.size(); ++i)
			{
				luups::to_lua(lua, vec[i]);
				lua.set_field(-2, detail::narrow_cast<lua_integer>(i + 1));
			}
		}
	};

	template<class T>
	struct from_lua_impl<std::vector<T>>
	{

		// note: iterates from index 1 until the first nil value
		// values are expected to be the correct type.

		static std::vector<T> from_lua(lua_state& lua, int index)
		{
			auto vec = std::vector<T>{ };

			for (auto i = lua_integer{ 1 }; ; ++i)
			{
				lua.push_field_raw(index, i);

				if (lua.is_nil(-1))
				{
					lua.pop();
					break;
				}

				vec.push_back(luups::from_lua<T>(lua));
				lua.pop();
			}

			return vec;
		}
	};

	// MAP

	template<class K, class V>
	struct to_lua_impl<std::map<K, V>>
	{
		static void to_lua(lua_state& lua, std::map<K, V> const& map)
		{
			lua.push_new_table();

			for (auto const& [key, value] : map)
			{
				luups::to_lua(lua, key);
				luups::to_lua(lua, value);
				lua.set_field(-3);
			}
		}
	};

	template<class K, class V>
	struct from_lua_impl<std::map<K, V>>
	{
		static std::map<K, V> from_lua(lua_state& lua, int index)
		{
			auto map = std::map<K, V>{ };

			lua.push_nil();

			while (lua.next_field(-2))
			{
				auto const k = luups::from_lua<K>(lua, -2);
				auto const v = luups::from_lua<V>(lua, -1);
				map[k] = v;

				lua.pop();
			}

			lua.pop();

			return map;
		}
	};

	// TUPLE

	template<class... Ts>
	struct to_lua_impl<std::tuple<Ts...>>
	{
		template<class T, int I>
		static void to_lua_value(lua_state& lua, T const& value)
		{
			luups::to_lua(lua, value);
			lua.set_field(-2, I);
		}

		template<class... Ts, int... Is>
		static void to_lua_values(lua_state& lua, std::tuple<Ts...> const& tup, std::integer_sequence<int, Is...>)
		{
			(to_lua_value<Ts, Is + 1>(lua, std::get<Is>(tup)), ...);
		}

		static void to_lua(lua_state& lua, std::tuple<Ts...> const& tup)
		{
			lua.push_new_table();

			to_lua_values(lua, tup, std::make_integer_sequence<int, sizeof...(Ts)>{ });
		}
	};

	template<class... Ts>
	struct from_lua_impl<std::tuple<Ts...>>
	{
		// note: iterates indices covering the entire tuple so holes (nil values)
		// are ignored, and filled with default constructed values.
		// if values are present, they are expected to be the correct type (for now).

		template<class T, int I>
		static T from_lua_value(lua_state& lua, int index)
		{
			lua.push_field_raw(index, I);

			if (lua.is_nil(-1))
			{
				lua.pop();
				return T{ };
			}

			auto const value = luups::from_lua<T>(lua, -1);
			lua.pop();
			return value;
		}

		template<class... Ts, int... Is>
		static std::tuple<Ts...> from_lua_values(lua_state& lua, int index, std::integer_sequence<int, Is...>)
		{
			return { from_lua_value<Ts, Is + 1>(lua, index)... };
		}

		static std::tuple<Ts...> from_lua(lua_state& lua, int index)
		{
			return from_lua_values<Ts...>(lua, index, std::make_integer_sequence<int, sizeof...(Ts)>{ });
		}
	};

	// todo: other std types?
	// todo: glm types

	namespace detail
	{

		template<class... Rets, int... Is>
		std::tuple<Rets...> tuple_from_lua(lua_state& lua, std::integer_sequence<int, Is...>)
		{
			static_assert(sizeof...(Rets) == sizeof...(Is));
			auto constexpr start = -static_cast<int>(sizeof...(Rets));
			return std::tuple<Rets...>{ from_lua<Rets>(lua, start + Is)... };
		}

		template<class... Rets>
		std::tuple<Rets...> tuple_from_lua(lua_state& lua)
		{
			return tuple_from_lua<Rets...>(lua, std::make_integer_sequence<int, sizeof...(Rets)>{ });
		}

		template<class... Rets, class... Args>
		auto run_loaded(lua_state& lua, Args&&... args)
		{
			(to_lua(lua, args), ...);

			auto constexpr num_args = sizeof...(Args);
			auto constexpr num_rets = sizeof...(Rets);

			if (lua.call(num_args, num_rets) != lua_status::ok)
				throw std::runtime_error(lua.pop_string());

			if constexpr (num_rets == 0)
			{
				return;
			}
			else if constexpr (num_rets == 1)
			{
				auto result = from_lua<Rets...>(lua, -1);
				lua.pop();
				return result;
			}
			else
			{
				auto results = tuple_from_lua<Rets...>(lua);
				lua.pop(num_rets);
				return results;
			}
		}

	} // detail

	template<class... Rets, class... Args>
	auto run(lua_state& lua, std::string const& code, Args&&... args)
	{
		if (lua.load_string(code) != lua_status::ok)
			throw std::runtime_error(lua.pop_string());
		
		return detail::run_loaded<Rets...>(lua, std::forward<Args>(args)...);
	}

	template<class... Rets, class... Args>
	auto frun(lua_state& lua, std::string const& file_path, Args&&... args)
	{
		if (lua.load_file(file_path) != lua_status::ok)
			throw std::runtime_error(lua.pop_string());
		
		return detail::run_loaded<Rets...>(lua, std::forward<Args>(args)...);
	}

	inline lua_state new_state()
	{
		return lua_state(luaL_newstate());
	}

	inline lua_state new_state(lua_debug_allocator& ud)
	{
		return lua_state(lua_newstate(debug_alloc, &ud));
	}

} // luups
