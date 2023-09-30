#pragma once

#include "bump_net_socket.hpp"

#include <cstdint>
#include <deque>
#include <span>
#include <vector>

namespace bump
{
	
	namespace net
	{
		
		class send_buffer
		{
		public:

			explicit send_buffer(std::size_t max_buffer_size) { m_buffer.reserve(max_buffer_size); }
			
			template<class It>
			void push_back(It first, It last);

			result<std::size_t, std::system_error> send(socket& connection);

			std::size_t size() const { return m_stream.size(); }
			bool empty() const { return size() == 0; }

			void clear() { m_stream.clear(); }

		private:
			
			std::deque<std::uint8_t> m_stream;
			std::vector<std::uint8_t> m_buffer;
		};
		
		template<class It>
		void send_buffer::push_back(It first, It last)
		{
			m_stream.insert(m_stream.end(), first, last);
		}
		
	} // net
	
} // bump
