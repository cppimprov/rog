#include "bump_net_send_buffer.hpp"

namespace bump
{
	
	namespace net
	{

		result<std::size_t, std::system_error> send_buffer::send(socket& connection)
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
