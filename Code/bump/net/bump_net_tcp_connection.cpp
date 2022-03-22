#include "bump_net_tcp_connection.hpp"

namespace bump
{
	
	namespace net
	{

		tcp_connection::tcp_connection(socket&& socket):
			m_socket(std::move(socket)) { }

		result<std::size_t, std::system_error> tcp_connection::send(std::span<const std::uint8_t> data)
		{
			auto const result = m_socket.send(data);

			if (!result.has_value())
				close();

			return result;
		}

		result<std::size_t, std::system_error> tcp_connection::receive(std::span<std::uint8_t> buffer)
		{
			auto const result = m_socket.receive(buffer);

			if (!result.has_value()) // error
			{
				close();
				return make_err(result.error());
			}

			if (!result.value().has_value()) // connection closed
			{
				close();
				return make_ok(std::size_t{ 0 });
			}
			
			return make_ok(result.value().value()); // received some data
		}

		bool tcp_connection::is_open() const
		{
			return m_socket.is_open();
		}

		result<void, std::system_error> tcp_connection::close()
		{
			return m_socket.close();
		}

		platform::socket_handle tcp_connection::get_handle() const
		{
			return m_socket.get_handle();
		}

		platform::socket_handle tcp_connection::release()
		{
			return m_socket.release();
		}

	} // net
	
} // bump
