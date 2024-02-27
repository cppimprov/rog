#pragma once

#include "bump_lua_defines.hpp"

#include <span>

namespace bump
{

	namespace lua
	{

		class state_handle
		{
		public:

			state_handle(): L(nullptr) { }
			explicit state_handle(state_t* L): L(L) { }

			state_handle(state_handle const&) = delete;
			state_handle& operator=(state_handle const&) = delete;

			state_handle(state_handle&& other): L(other.L) { other.L = nullptr; }
			state_handle& operator=(state_handle&& other) { auto temp = std::move(other); std::swap(L, temp.L); return *this; }

			~state_handle() { close(); }

			bool is_open() const { return L != nullptr; }
			void close() { if (is_open()) lua_close(L); L = nullptr; }

			void reset() { close(); }
			void reset(state_t* new_L) { close(); L = new_L; }

			state_t* handle() const { return L; }
			state_t* release() { auto old_L = L; L = nullptr; return old_L; }

		private:

			state_t* L;
		};
		
		class state_view
		{
		public:

			// CONSTRUCT / DESTRUCT

			state_view(): L(nullptr) { }
			explicit state_view(state_t* L): L(L) { }

			state_view(state_view const&) = default;
			state_view& operator=(state_view const&) = default;
			state_view(state_view&&) = default;
			state_view& operator=(state_view&&) = default;

			// STATE

			bool is_open() const { return L != nullptr; }
			state_t* handle() const { return L; }

			// UTILS

			number_t version() const;

			cfunction_t at_panic(cfunction_t panic_fn);
			void error();
			void warn(std::string const& msg);
			status status() const;

			int dump(writer_t writer, void* ud = nullptr, bool strip_debug_info = false);

			load_mode set_load_mode(load_mode mode);
			int set_msg_handler(int index);

			// MEMORY

			void set_allocator(allocator_t alloc, void* ud);
			allocator_t get_allocator(void** ud) const;

			void gc_collect();
			void gc_step(int step_size_kb);
			void gc_stop();
			void gc_restart();
			bool gc_is_running() const;
			gc_mode gc_inc(int pause, int step_mul, int step_size);
			gc_mode gc_gen(int minor_mul, int major_mul);
			int gc_count_bytes() const;

			// LIBRARIES / PACKAGES

			void open_libraries();
			void open_std_library(std_library library);
			void require(std::string const& module_name, cfunction_t open_fn, bool global = true);
			void register_fn(std::string const& name, cfunction_t fn);
			void new_library(std::span<reg_t const> fns, int num_upvalues);
			void register_fns(std::span<reg_t const> fns, int num_upvalues);

			// LOAD / EXECUTE
			
			[[nodiscard]] lua::status load(reader_t reader, std::string const& chunk_name = "?", void* ud = nullptr);
			[[nodiscard]] lua::status load_string(std::string const& code, std::string const& chunk_name = "?");
			[[nodiscard]] lua::status load_file(std::string const& path);
			[[nodiscard]] lua::status load_stdin();
			
			[[nodiscard]] lua::status call(int num_args, int num_results);
			[[nodiscard]] void call_unprotected(int num_args, int num_results);

			[[nodiscard]] lua::status do_string(std::string const& code, std::string const& chunk_name = "?");
			[[nodiscard]] lua::status do_file(std::string const& path);

			// STACK - MANAGEMENT

			int size() const;
			bool empty() const { return size() == 0; }
			void resize(int index);
			void clear() { resize(0); }

			void copy(int from_index, int to_index);
			void push_copy(int index);
			void insert(int index);
			void pop(int num_elements = 1);
			void remove(int index);
			void replace(int index);
			void rotate(int index, int num_elements);
			void concat(int num_elements);
			void gsub(std::string const& str, std::string const& pattern, std::string const& replacement);
			
			[[nodiscard]] bool check(int num_elements);
			void check_stack(int num_elements, std::string const& msg = "");
			int to_abs_index(int index) const;

			// TYPES

			// note: invalid indices (outside the currently allocated stack space) return none
			// note: indices that are valid, but currently empty return nil
			type get_type(int index) const { return static_cast<lua::type>(lua_type(L, index)); }

			bool is_none(int index) const { return get_type(index) == type::none; }
			bool is_nil(int index) const { return get_type(index) == type::nil; }
			bool is_none_or_nil(int index) const { return is_none(index) || is_nil(index); }
			bool is_boolean(int index) const { return get_type(index) == type::boolean; }
			bool is_integer(int index) const { return get_type(index) == type::number && lua_isinteger(L, index); }
			bool is_number(int index) const { return get_type(index) == type::number; }
			bool is_string(int index) const { return get_type(index) == type::string; }
			bool is_table(int index) const { return get_type(index) == type::table; }
			bool is_function(int index) const { return get_type(index) == type::function; }
			bool is_cfunction(int index) const { return get_type(index) == type::function && lua_iscfunction(L, index); }
			bool is_userdata(int index) const { return get_type(index) == type::userdata; }
			bool is_lightuserdata(int index) const { return get_type(index) == type::lightuserdata; }
			bool is_thread(int index) const { return get_type(index) == type::thread; }

			void check_none(int index) const;
			void check_nil(int index) const;
			void check_boolean(int index) const;
			void check_integer(int index) const;
			void check_number(int index) const;
			void check_string(int index) const;
			void check_table(int index) const;
			void check_function(int index) const;
			void check_cfunction(int index) const;
			void check_userdata(int index) const;
			void check_lightuserdata(int index) const;
			void check_thread(int index) const;

			// VARIABLES

			void push_nil();
			void push_fail();
			void push_boolean(bool value);
			void push_integer(integer_t value);
			void push_number(number_t value);
			void push_string(std::string_view value);
			void push_cclosure(cfunction_t value, int num_values);
			void push_cfunction(cfunction_t value);
			void push_lightuserdata(void* value);
			void push_fulluserdata(std::size_t block_size, int num_user_values);

			template<class... Args>
			void push_fstring(std::string const& format, Args&&... args) { lua_pushfstring(L, format.data(), std::forward<Args>(args)...); }

			bool to_boolean(int index = -1) const;
			integer_t to_integer(int index = -1) const;
			number_t to_number(int index = -1) const;
			std::string_view to_string(int index = -1) const;
			cfunction_t to_cfunction(int index = -1) const;
			void* to_userdata(int index = -1) const;

			bool pop_boolean(int index = -1) { auto const value = to_boolean(index); pop(); return value; }
			integer_t pop_integer(int index = -1) { auto const value = to_integer(index); pop(); return value; }
			number_t pop_number(int index = -1) { auto const value = to_number(index); pop(); return value; }
			std::string pop_string(int index = -1) { auto const value = std::string(to_string(index)); pop(); return value; }
			cfunction_t pop_cfunction(int index = -1) { auto const value = to_cfunction(index); pop(); return value; }
			void* pop_userdata(int index = -1) { auto const value = to_userdata(index); pop(); return value; }

			integer_t get_length(int index = -1);
			unsigned_t get_length_raw(int index = -1);
			void push_length(int index = -1);

			// GENERIC PUSH / TO / POP

			template<class T>
			void push(T&& value) { to_lua(*this, std::forward<T>(value)); }

			template<class T>
			T to(int index = -1) { return from_lua<T>(*this, index); }

			template<class T>
			T pop(int index = -1) { auto const value = to<T>(index); pop(); return value; }

			// TABLES

			void push_new_table(int sequence_size = 0, int hash_size = 0);
			void push_globals_table();

			type push_field(int table_index);
			type push_field_raw(int table_index);
			type push_field(int table_index, std::string const& key_name);
			type push_field_raw(int table_index, std::string const& key_name);
			type push_field(int table_index, integer_t key_index);
			type push_field_raw(int table_index, integer_t key_index);
			type push_field_raw(int table_index, void* key);
			type push_global(std::string const& key_name);

			void set_field(int table_index);
			void set_field_raw(int table_index);
			void set_field(int table_index, std::string const& key_name);
			void set_field_raw(int table_index, std::string const& key_name);
			void set_field(int table_index, integer_t key_index);
			void set_field_raw(int table_index, integer_t key_index);
			void set_field_raw(int table_index, void* key);
			void set_global(std::string const& key_name);

			bool next_field(int table_index);

			// USERDATA

			void push_userdata_value(int userdata_index, int value_index);
			void set_userdata_value(int userdata_index, int value_index);

			// REFERENCES

			int set_ref(int table_index);
			void clear_ref(int table_index, int ref);
			
			// OPERATIONS

			void arith(arith_op op);

			bool compare(compare_op op, int index1, int index2);
			bool compare_eq(int index1, int index2);
			bool compare_lt(int index1, int index2);
			bool compare_le(int index1, int index2);

			bool raw_eq(int index1, int index2);

		private:

			// THE STATE

			state_t* L;

			// todo: these shouldn't be here? we end up with different message handlers / load modes for 
			// state views that are supposed to be the same state :(

			// LOADING

			load_mode load_mode = binary | text;

			// MESSAGE HANDLER
			
			int msg_handler_idx = lua_no_msg_handler;
		};
		
		int throw_runtime_error(state_t* L);
		
		class state : public state_handle, public state_view
		{
		public:

			state(): state_handle(), state_view() { }

			using state_handle::is_open;
			using state_handle::handle;

		private:

			// this is private to prevent state(L) being used by accident instead of state_view(L)
			explicit state(state_t* L): state_handle(L), state_view(L) { if (is_open()) at_panic(&throw_runtime_error); }

			friend state new_state();
			friend state new_state(allocator_t alloc, void* ud);
		};
		
		inline state new_state()
		{
			return state(luaL_newstate());
		}

		inline state new_state(allocator_t alloc, void* ud)
		{
			return state(lua_newstate(alloc, ud));
		}

	} // lua

} // bump
