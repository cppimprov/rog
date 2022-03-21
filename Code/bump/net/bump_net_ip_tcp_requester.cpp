#include "bump_net_ip_tcp_requester.hpp"

#include "bump_net_ip_endpoint.hpp"

namespace bump
{
	
	namespace net
	{
		
		namespace ip
		{
			
			tcp_requester::tcp_requester(socket&& socket):
				m_socket(std::move(socket)) { }
			
			result<tcp_connection, std::system_error> tcp_requester::check()
			{
				auto const result = m_socket.check();

				if (!result.has_value())
					return make_err(result.error()); // error while checking!
				
				if (!result.value().has_value())
					return make_ok(tcp_connection()); // still waiting...
				
				if (!result.value().value())
				{
					cancel();
					return make_ok(tcp_connection()); // connection failed
				}

				return make_ok(tcp_connection(std::move(m_socket))); // success!
			}

			bool tcp_requester::is_active() const
			{
				return m_socket.is_open();
			}

			result<void, std::system_error> tcp_requester::cancel()
			{
				return m_socket.close();
			}

			platform::socket_handle tcp_requester::get_handle() const
			{
				return m_socket.get_handle();
			}

			platform::socket_handle tcp_requester::release()
			{
				return m_socket.release();
			}

			result<tcp_requester, std::system_error> connect(endpoint const& endpoint, blocking_mode mode)
			{
				auto socket_result = open_socket(endpoint.get_address_family(), ip::protocol::TCP);

				if (!socket_result.has_value())
					return make_err(socket_result.error());
				
				auto socket = socket_result.unwrap();

				auto blocking_mode_result = socket.set_blocking_mode(mode);

				if (!blocking_mode_result)
					return make_err(blocking_mode_result.error());

				auto connect_result = socket.connect(endpoint);

				if (!connect_result.has_value())
					return make_err(connect_result.error());
				
				return make_ok(tcp_requester(std::move(socket)));
			}

		} // ip
		
	} // net
	
} // bump
