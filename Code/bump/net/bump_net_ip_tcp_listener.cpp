#include "bump_net_ip_tcp_listener.hpp"

#include "bump_net_ip_endpoint.hpp"

namespace bump
{
	
	namespace net
	{

		namespace ip
		{
		
			tcp_listener::tcp_listener(socket&& socket):
				m_socket(std::move(socket)) { }

			result<tcp_connection, std::system_error> tcp_listener::accept() const
			{
				auto result = m_socket.accept();

				if (!result.has_value())
					return make_err(result.error());
				
				return make_ok(tcp_connection(result.unwrap()));
			}

			bool tcp_listener::is_active() const
			{
				return m_socket.is_open();
			}

			result<void, std::system_error> tcp_listener::cancel()
			{
				return m_socket.close();
			}

			platform::socket_handle tcp_listener::get_handle() const
			{
				return m_socket.get_handle();
			}

			platform::socket_handle tcp_listener::release()
			{
				return m_socket.release();
			}

			result<tcp_listener, std::system_error> listen(ip::endpoint const& endpoint)
			{
				auto socket_result = open_socket(endpoint.get_address_family(), ip::protocol::TCP);

				if (!socket_result.has_value())
					return make_err(socket_result.error());

				auto socket = socket_result.unwrap();
			
				auto bind_result = socket.bind(endpoint);

				if (!bind_result.has_value())
					return make_err(bind_result.error());
				
				auto listen_result = socket.listen();

				if (!listen_result.has_value())
					return make_err(listen_result.error());
				
				return make_ok(tcp_listener(std::move(socket)));
			}
			
		} // ip
		
	} // net
	
} // bump
