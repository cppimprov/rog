#pragma once

#include "bump_net_tcp_connection.hpp"
#include "bump_net_socket.hpp"
#include "bump_result.hpp"

#include <system_error>

namespace bump
{
	
	namespace net
	{
		
		class tcp_requester
		{
		public:

			tcp_requester() = default;
			explicit tcp_requester(socket&& socket);

			tcp_requester(tcp_requester const&) = delete;
			tcp_requester operator=(tcp_requester const&) = delete;

			tcp_requester(tcp_requester&& other) = default;
			tcp_requester& operator=(tcp_requester&& other) = default;

			result<tcp_connection, std::system_error> check();

			bool is_active() const;
			result<void, std::system_error> cancel();

			platform::socket_handle get_handle() const;
			platform::socket_handle release();

		private:

			socket m_socket;
		};

		result<tcp_requester, std::system_error> connect(endpoint const& endpoint, blocking_mode mode);
			
	} // net
	
} // bump
