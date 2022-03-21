#pragma once

#include "bump_net_socket.hpp"
#include "bump_result.hpp"

#include <system_error>

namespace bump
{
	
	namespace net
	{

		namespace ip
		{

			class tcp_connection
			{
			public:

				tcp_connection();
				explicit tcp_connection(socket&& socket);

				tcp_connection(tcp_connection const&) = delete;
				tcp_connection& operator=(tcp_connection const&) = delete;
				
				tcp_connection(tcp_connection&&) = default;
				tcp_connection& operator=(tcp_connection&&) = default;

				result<std::size_t, std::system_error> send(std::uint8_t const* data, std::size_t data_size);
				result<std::size_t, std::system_error> receive(std::uint8_t* data, std::size_t data_size);

				bool is_open() const;
				result<void, std::system_error> close();

				platform::socket_handle get_handle() const;
				platform::socket_handle release();

			private:

				socket m_socket;
			};

		} // ip
		
	} // net
	
} // bump
