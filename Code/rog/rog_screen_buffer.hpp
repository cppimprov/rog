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

		struct cell
		{
			char m_value;
			glm::vec3 m_fg;
			glm::vec3 m_bg;
		};

		screen_buffer() = default;

		explicit screen_buffer(glm::size2 size): m_data(size) { }
		explicit screen_buffer(glm::size2 size, cell const& cell): m_data(size, cell) { }

		screen_buffer(screen_buffer const&) = default;
		screen_buffer& operator=(screen_buffer const&) = default;

		screen_buffer(screen_buffer &&) = default;
		screen_buffer& operator=(screen_buffer &&) = default;

		glm::size2 get_size() const { return m_data.extents(); }
		std::size_t get_data_size() const { return m_data.size(); }

		void resize(glm::size2 size, cell const& cell);

		void clear();
		
		void fill_rect(glm::size2 origin, glm::size2 size, cell const& cell);
		void print_char(glm::size2 origin, cell const& cell);
		void print_string(glm::size2 origin, std::string const& s, glm::vec3 fg, glm::vec3 bg);
		
		cell& at(glm::size2 coords) { return m_data.at(coords); }
		cell const& at(glm::size2 coords) const { return m_data.at(coords); }

	private:

		bump::grid2<cell> m_data;
	};
	
} // rog
