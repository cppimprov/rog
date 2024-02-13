#pragma once

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

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
		// todo: check if this matches lua's GCCOUNT and GCCOUNTB

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

		[[nodiscard]] lua_status load_string(std::string const& code, lua_load_mode mode = binary | text);
		[[nodiscard]] lua_status load_file(std::string const& path, lua_load_mode mode = binary | text);
		[[nodiscard]] lua_status call(int num_args, int num_results = lua_multiple_return, int msg_handler_idx = lua_no_msg_handler);
		[[nodiscard]] lua_status do_string(std::string const& code, lua_load_mode mode = binary | text, int num_results = lua_multiple_return, int msg_handler_idx = lua_no_msg_handler);
		[[nodiscard]] lua_status do_file(std::string const& path, lua_load_mode mode = binary | text, int num_results = lua_multiple_return, int msg_handler_idx = lua_no_msg_handler);

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
		lua_type push_field(int table_index, std::uint64_t key_index);
		lua_type push_field_raw(int table_index, std::uint64_t key_index);
		lua_type push_global(std::string const& key_name);

		void set_field(int table_index);
		void set_field_raw(int table_index);
		void set_field(int table_index, std::string const& key_name);
		void set_field_raw(int table_index, std::string const& key_name);
		void set_field(int table_index, std::uint64_t key_index);
		void set_field_raw(int table_index, std::uint64_t key_index);
		void set_global(std::string const& key_name);

		int next_field(int table_index);

		// STACK - REFERENCES

		int set_ref(int table_index);
		void clear_ref(int table_index, int ref);

	private:

		// THE STATE

		lua_State* L;

		// ERROR HANDLING

		static int default_panic_fn(lua_State* L);

		// STACK MANAGEMENT UTILS

		bool to_boolean_impl(int index) const;
		lua_integer to_integer_impl(int index) const;
		lua_number to_number_impl(int index) const;
		std::string_view to_string_impl(int index) const;
		lua_cfunction to_cfunction_impl(int index) const;
	};
	

	template<class T> struct to_lua_impl;

	template<class T>
	void to_lua(lua_state& state, T const& value)
	{
		return to_lua_impl<std::decay_t<T>>::to_lua(state, value);
	}

	template<class T> struct from_lua_impl;

	template<class T>
	T from_lua(lua_state& state)
	{
		return from_lua_impl<std::decay_t<T>>::from_lua(state);
	}
	
	template<>
	struct to_lua_impl<bool>
	{
		static void to_lua(lua_state& state, bool value) { state.push_boolean(value); }
	};

	template<>
	struct from_lua_impl<bool>
	{
		static bool from_lua(lua_state& state) { return state.pop_boolean(); }
	};

	template<>
	struct to_lua_impl<lua_integer>
	{
		static void to_lua(lua_state& state, lua_integer value) { state.push_integer(value); }
	};

	template<>
	struct from_lua_impl<lua_integer>
	{
		static lua_integer from_lua(lua_state& state) { return state.pop_integer(); }
	};

	template<>
	struct to_lua_impl<lua_number>
	{
		static void to_lua(lua_state& state, lua_number value) { state.push_number(value); }
	};

	template<>
	struct from_lua_impl<lua_number>
	{
		static lua_number from_lua(lua_state& state) { return state.pop_number(); }
	};

	template<>
	struct to_lua_impl<std::string>
	{
		static void to_lua(lua_state& state, std::string const& value) { state.push_string(value); }
	};

	template<>
	struct from_lua_impl<std::string>
	{
		static std::string from_lua(lua_state& state) { return state.pop_string(); }
	};

	namespace detail
	{

		template<class... Ts> struct reverse_tuple;

		template<>
		struct reverse_tuple<std::tuple<>>
		{
			using type = std::tuple<>;
		};

		template<class T, class... Ts>
		struct reverse_tuple<std::tuple<T, Ts...>>
		{
			using head = std::tuple<T>;
			using tail = typename reverse_tuple<std::tuple<Ts...>>::type;
			using type = decltype(std::tuple_cat(std::declval<tail>(), std::declval<head>()));
		};

		template<class T>
		using reverse_tuple_t = typename reverse_tuple<T>::type;

		template<class... Ts>
		std::tuple<Ts...> tuple_from_lua(lua_state& state, std::tuple<Ts...>)
		{
			return std::tuple{ from_lua<Ts>(state)... };
		}

		template<class... Ts>
		reverse_tuple_t<std::tuple<Ts...>> reverse_from_lua(lua_state& state)
		{
			return tuple_from_lua(state, reverse_tuple_t<std::tuple<Ts...>>{ });
		}

		template<class T, std::size_t... Is>
		reverse_tuple_t<T> reverse_impl(T&& t, std::index_sequence<Is...>)
		{
			return { std::get<sizeof...(Is) - 1 - Is>(std::forward<T>(t))... };
		}

		template<class T>
		reverse_tuple_t<T> reverse(T&& t)
		{
			return reverse_impl(std::forward<T>(t), std::make_index_sequence<std::tuple_size_v<T>>{ });
		}

	} // detail

	template<class... Rets, class... Args>
	decltype(auto) run(lua_state& state, std::string const& code, [[maybe_unused]] Args&&... args)
	{
		if (state.load_string(code) != lua_status::ok)
			throw std::runtime_error(state.pop_string());
		
		(to_lua(state, args), ...);

		auto constexpr num_args = sizeof...(Args);
		auto constexpr num_rets = sizeof...(Rets);

		if (state.call(num_args, num_rets) != lua_status::ok)
			throw std::runtime_error(state.pop_string());

		if constexpr (num_rets == 0)
		{
			return;
		}
		else if constexpr (num_rets == 1)
		{
			return (from_lua<Rets>(state), ...);
		}
		else
		{
			// note: from_lua (and hence pop) must be called for the *last*
			// argument in Rets... *first*. the resulting tuple must then be
			// reversed to get back to the original order of Rets...
			return detail::reverse(detail::reverse_from_lua<Rets...>(state));
		}
	}

	// todo: frun() - like run(), but takes a file path, not a code string
	// todo: keep a default lua_load_mode in the lua_state?

	inline lua_state new_state()
	{
		return lua_state(luaL_newstate());
	}

	inline lua_state new_state(lua_debug_allocator& ud)
	{
		return lua_state(lua_newstate(debug_alloc, &ud));
	}

} // luups
