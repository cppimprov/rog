#pragma once

#include "bump_lua_io.hpp"
#include "bump_lua_detail_narrow_cast.hpp"

namespace bump
{

	namespace lua
	{

		// BOOL
	
		template<>
		struct to_lua_impl<bool>
		{
			static void to_lua(state_view lua, bool value) { lua.push_boolean(value); }
		};

		template<>
		struct from_lua_impl<bool>
		{
			static bool from_lua(state_view lua, int index) { return lua.to_boolean(index); }
		};


		// INTEGERS

		template<> struct to_lua_impl<char>               { static void to_lua(state_view lua, char value) {               lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<signed char>        { static void to_lua(state_view lua, signed char value) {        lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<unsigned char>      { static void to_lua(state_view lua, unsigned char value) {      lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<short>              { static void to_lua(state_view lua, short value) {              lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<unsigned short>     { static void to_lua(state_view lua, unsigned short value) {     lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<int>                { static void to_lua(state_view lua, int value) {                lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<unsigned int>       { static void to_lua(state_view lua, unsigned int value) {       lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<long>               { static void to_lua(state_view lua, long value) {               lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<unsigned long>      { static void to_lua(state_view lua, unsigned long value) {      lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<long long>          { static void to_lua(state_view lua, long long value) {          lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<unsigned long long> { static void to_lua(state_view lua, unsigned long long value) { lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<char8_t>            { static void to_lua(state_view lua, char8_t value) {            lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<char16_t>           { static void to_lua(state_view lua, char16_t value) {           lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<char32_t>           { static void to_lua(state_view lua, char32_t value) {           lua.push_integer(detail::narrow_cast<integer_t>(value)); } };
		template<> struct to_lua_impl<wchar_t>            { static void to_lua(state_view lua, wchar_t value) {            lua.push_integer(detail::narrow_cast<integer_t>(value)); } };

		template<> struct from_lua_impl<char>               { static char from_lua(state_view lua, int index) {               return detail::narrow_cast<char>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<signed char>        { static signed char from_lua(state_view lua, int index) {        return detail::narrow_cast<signed char>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<unsigned char>      { static unsigned char from_lua(state_view lua, int index) {      return detail::narrow_cast<unsigned char>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<short>              { static short from_lua(state_view lua, int index) {              return detail::narrow_cast<short>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<unsigned short>     { static unsigned short from_lua(state_view lua, int index) {     return detail::narrow_cast<unsigned short>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<int>                { static int from_lua(state_view lua, int index) {                return detail::narrow_cast<int>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<unsigned int>       { static unsigned int from_lua(state_view lua, int index) {       return detail::narrow_cast<unsigned int>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<long>               { static long from_lua(state_view lua, int index) {               return detail::narrow_cast<long>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<unsigned long>      { static unsigned long from_lua(state_view lua, int index) {      return detail::narrow_cast<unsigned long>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<long long>          { static long long from_lua(state_view lua, int index) {          return detail::narrow_cast<long long>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<unsigned long long> { static unsigned long long from_lua(state_view lua, int index) { return detail::narrow_cast<unsigned long long>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<char8_t>            { static char8_t from_lua(state_view lua, int index) {            return detail::narrow_cast<char8_t>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<char16_t>           { static char16_t from_lua(state_view lua, int index) {           return detail::narrow_cast<char16_t>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<char32_t>           { static char32_t from_lua(state_view lua, int index) {           return detail::narrow_cast<char32_t>(lua.to_integer(index)); } };
		template<> struct from_lua_impl<wchar_t>            { static wchar_t from_lua(state_view lua, int index) {            return detail::narrow_cast<wchar_t>(lua.to_integer(index)); } };


		// FLOATING POINTS

		template<> struct to_lua_impl<float>       { static void to_lua(state_view lua, float value) {       lua.push_number(detail::narrow_cast<number_t>(value)); } };
		template<> struct to_lua_impl<double>      { static void to_lua(state_view lua, double value) {      lua.push_number(detail::narrow_cast<number_t>(value)); } };
		template<> struct to_lua_impl<long double> { static void to_lua(state_view lua, long double value) { lua.push_number(detail::narrow_cast<number_t>(value)); } };

		template<> struct from_lua_impl<float>       { static float from_lua(state_view lua, int index) {       return detail::narrow_cast<float>(lua.to_number(index)); } };
		template<> struct from_lua_impl<double>      { static double from_lua(state_view lua, int index) {      return detail::narrow_cast<double>(lua.to_number(index)); } };
		template<> struct from_lua_impl<long double> { static long double from_lua(state_view lua, int index) { return detail::narrow_cast<long double>(lua.to_number(index)); } };

	} // lua

} // bump
