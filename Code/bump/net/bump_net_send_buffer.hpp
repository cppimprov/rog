#pragma once

#include "bump_net_ip_tcp_connection.hpp"

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

			result<std::size_t, std::system_error> send(ip::tcp_connection& connection);

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

		result<std::size_t, std::system_error> send_buffer::send(ip::tcp_connection& connection)
		{
			auto const size = std::min(m_stream.size(), m_buffer.capacity());

			m_buffer.resize(size);
			std::copy_n(m_stream.begin(), size, m_buffer.begin());

			auto bytes_sent = connection.send(std::span<const std::uint8_t>(m_buffer.data(), size));

			if (!bytes_sent.has_value())
				return make_err(bytes_sent.error());

			m_buffer.clear();
			m_stream.erase(m_stream.begin(), m_stream.begin() + bytes_sent.value());

			return make_ok(bytes_sent.value());
		}
		
	} // net
	
} // bump