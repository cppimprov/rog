#pragma once

#include "bump_net_tcp_connection.hpp"
#include "bump_net_socket.hpp"
#include "bump_result.hpp"

#include <system_error>
#include <tuple>

namespace bump
{
	
	namespace net
	{

		class endpoint;

		struct tcp_listener
		{
		public:

			tcp_listener() = default;
			explicit tcp_listener(socket&& socket);

			tcp_listener(tcp_listener const&) = delete;
			tcp_listener operator=(tcp_listener const&) = delete;

			tcp_listener(tcp_listener&& other) = default;
			tcp_listener& operator=(tcp_listener&& other) = default;

			result<std::tuple<tcp_connection, endpoint>, std::system_error> accept() const;

			bool is_active() const;
			result<void, std::system_error> cancel();

			platform::socket_handle get_handle() const;
			platform::socket_handle release();

		private:

			socket m_socket;
		};

		result<tcp_listener, std::system_error> listen(endpoint const& endpoint, blocking_mode mode);

	} // net
	
} // bump
