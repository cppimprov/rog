#pragma once

#include <bump_grid.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/std_based_type.hpp>

#include <string>

namespace rog
{
	
	class screen_buffer
	{
	public:

		screen_buffer() = default;

		explicit screen_buffer(glm::size2 size): m_data(size) { }
		explicit screen_buffer(glm::size2 size, char value): m_data(size, value) { }

		screen_buffer(screen_buffer const&) = default;
		screen_buffer& operator=(screen_buffer const&) = default;

		screen_buffer(screen_buffer &&) = default;
		screen_buffer& operator=(screen_buffer &&) = default;

		glm::size2 get_size() const { return m_data.extents(); }

		void clear();
		
		void fill_rect(glm::size2 origin, glm::size2 size, char value);
		void print_char(glm::size2 origin, char value);
		void print_string(glm::size2 origin, std::string const& s);
		
		char& at(glm::size2 coords) { return m_data.at(coords); }
		char const& at(glm::size2 coords) const { return m_data.at(coords); }

	private:

		bump::grid2<char> m_data;
	};
	
} // rog
