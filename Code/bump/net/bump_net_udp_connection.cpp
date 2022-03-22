#include "bump_net_udp_connection.hpp"

namespace bump
{

	namespace net
	{
	
		udp_connection::udp_connection(socket&& socket):
			m_socket(std::move(socket)) { }
		
		result<std::size_t, std::system_error> udp_connection::send(std::span<const std::uint8_t> data)
		{
			auto const result = m_socket.send(data);

			if (!result.has_value())
				close();

			return result;
		}

		result<std::size_t, std::system_error> udp_connection::receive(std::span<std::uint8_t> buffer)
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

		bool udp_connection::is_open() const
		{
			return m_socket.is_open();
		}

		result<void, std::system_error> udp_connection::close()
		{
			return m_socket.close();
		}

		platform::socket_handle udp_connection::get_handle() const
		{
			return m_socket.get_handle();
		}

		platform::socket_handle udp_connection::release()
		{
			return m_socket.release();
		}

		result<udp_connection, std::system_error> connect_udp(endpoint const& local, endpoint const& remote, blocking_mode mode)
		{
			auto socket_result = open_socket(local.get_address_family(), ip_protocol::UDP);

			if (!socket_result.has_value())
				return make_err(socket_result.error());
			
			auto socket = socket_result.unwrap();

			auto blocking_mode_result = socket.set_blocking_mode(mode);

			if (!blocking_mode_result.has_value())
				return make_err(blocking_mode_result.error());
			
			auto bind_result = socket.bind(local);

			if (!bind_result.has_value())
				return make_err(bind_result.error());
			
			auto connect_result = socket.connect(remote);

			if (!connect_result.has_value())
				return make_err(connect_result.error());
			
			return make_ok(udp_connection(std::move(socket)));
		}
		
	} // net

} // bump
