#pragma once

#include "bump_io_read_write.hpp"
#include "bump_math.hpp"
#include "bump_range.hpp"

namespace bump
{

	namespace io
	{
	
#pragma region vec

		template<size_t S, class T, glm::qualifier Q>
		struct write_impl<glm::vec<S, T, Q>>
		{
			static void write(std::ostream& os, glm::vec<S, T, Q> const& value)
			{
				for (auto i : range(0, S))
					io::write<T>(os, value[i]);
			}
		};

		template<size_t S, class T, glm::qualifier Q>
		struct read_impl<glm::vec<S, T, Q>>
		{
			static glm::vec<S, T, Q> read(std::istream& is)
			{
				auto value = glm::vec<S, T, Q>();

				for (auto i : range(0, S))
					value[i] = io::read<T>(is);
				
				return value;
			}
		};

#pragma endregion

#pragma region quat
		
		template<class T, glm::qualifier Q>
		struct write_impl<glm::qua<T, Q>>
		{
			static void write(std::ostream& os, glm::qua<T, Q> const& value)
			{
				for (auto i : range(0, 4))
					io::write<T>(os, value[i]);
			}
		};

		template<class T, glm::qualifier Q>
		struct read_impl<glm::qua<T, Q>>
		{
			static glm::qua<T, Q> read(std::istream& is)
			{
				auto value = glm::qua<T, Q>();

				for (auto i : range(0, 4))
					value[i] = io::read<T>(is);
				
				return value;
			}
		};

#pragma endregion

#pragma region mat

		template<size_t C, size_t R, class T, glm::qualifier Q>
		struct write_impl<glm::mat<C, R, T, Q>>
		{
			static void write(std::ostream& os, glm::mat<C, R, T, Q> const& value)
			{
				for (auto i : range(0, C))
					io::write<glm::mat<C, R, T, Q>::col_type>(os, value[i]);
			}
		};

		template<size_t C, size_t R, class T, glm::qualifier Q>
		struct read_impl<glm::mat<C, R, T, Q>>
		{
			static glm::mat<C, R, T, Q> read(std::istream& is)
			{
				auto value = glm::mat<C, R, T, Q>();

				for (auto i : range(0, C))
					value[i] = io::read<glm::mat<C, R, T, Q>::col_type>(is);
				
				return value;
			}
		};

#pragma endregion
	
	} // io

} // bump
