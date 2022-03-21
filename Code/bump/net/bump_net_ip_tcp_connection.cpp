#include "bump_net_ip_tcp_connection.hpp"

namespace bump
{
	
	namespace net
	{
		
		namespace ip
		{
			
			tcp_connection::tcp_connection(socket&& socket):
				m_socket(std::move(socket)) { }

			result<std::size_t, std::system_error> tcp_connection::send(std::uint8_t const* data, std::size_t data_size)
			{
				auto const result = net::send(m_socket, data, data_size);

				if (!result.has_value())
					close();

				return result;
			}

			result<std::size_t, std::system_error> tcp_connection::receive(std::uint8_t* data, std::size_t data_size)
			{
				auto const result = net::receive(m_socket, data, data_size);

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

		} // ip
		
	} // net
	
} // bump
