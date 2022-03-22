#pragma once

#include "bump_net_socket.hpp"
#include "bump_net_ip_endpoint.hpp"
#include "bump_result.hpp"

#include <system_error>

namespace bump
{

	namespace net
	{

		namespace ip
		{

			class udp_connection
			{
			public:

				udp_connection() = default;
				explicit udp_connection(socket&& socket);

				udp_connection(udp_connection const&) = delete;
				udp_connection operator=(udp_connection const&) = delete;

				udp_connection(udp_connection&& other) = default;
				udp_connection& operator=(udp_connection&& other) = default;

				result<std::size_t, std::system_error> send(std::span<const std::uint8_t> data);
				result<std::size_t, std::system_error> receive(std::span<std::uint8_t> buffer);

				bool is_open() const;
				result<void, std::system_error> close();

				platform::socket_handle get_handle() const;
				platform::socket_handle release();

			private:

				socket m_socket;
			};

			result<udp_connection, std::system_error> connect_udp(endpoint const& local, endpoint const& remote, blocking_mode mode);

		} // ip

	} // net

} // bump
