#pragma once

#include "bump_net_ip_enums.hpp"
#include "bump_net_platform.hpp"
#include "bump_result.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <system_error>
#include <tuple>

namespace bump
{
	
	namespace net
	{

		class endpoint;

		namespace platform
		{

#if defined(BUMP_NET_WS2)

			using socket_handle = SOCKET;
			auto constexpr invalid_socket_handle = INVALID_SOCKET;

#else

			using socket_handle = int;
			auto constexpr invalid_socket_handle = -1;

#endif

		} // platform

		enum class blocking_mode
		{
			BLOCKING,
			NON_BLOCKING,
		};
		
		struct select_result
		{
			bool readable, writeable, exceptional;
		};

		class socket
		{
		public:

			socket();
			explicit socket(platform::socket_handle handle);

			socket(socket const&) = delete;
			socket& operator=(socket const&) = delete;

			socket(socket&& other);
			socket& operator=(socket&& other);

			~socket();
			
			result<void, std::system_error> set_blocking_mode(blocking_mode mode) const;
			result<void, std::system_error> bind(endpoint const& local) const;
			result<void, std::system_error> listen() const;
			result<void, std::system_error> connect(endpoint const& remote) const;
			result<std::tuple<socket, endpoint>, std::system_error> accept() const;
			result<select_result, std::system_error> select() const;
			result<std::size_t, std::system_error> send(std::span<std::uint8_t const> data) const;
			result<std::size_t, std::system_error> send_to(endpoint const& remote, std::span<std::uint8_t const> data) const;
			result<std::size_t, std::system_error> receive(std::span<std::uint8_t> buffer) const;
			result<std::tuple<endpoint, std::size_t>, std::system_error> receive_from(std::span<std::uint8_t> buffer) const;
			result<endpoint, std::system_error> get_remote_endpoint() const;
			result<endpoint, std::system_error> get_local_endpoint() const;

			bool is_open() const;
			result<void, std::system_error> close();

			platform::socket_handle get_handle() const;
			platform::socket_handle release();
			
		private:

			platform::socket_handle m_handle;
		};

		result<socket, std::system_error> make_socket(ip_address_family address_family, ip_protocol protocol);
		result<socket, std::system_error> make_tcp_listener(endpoint const& local, blocking_mode mode);
		result<socket, std::system_error> make_tcp_stream(endpoint const& remote, blocking_mode mode);
		result<socket, std::system_error> make_udp_socket(endpoint const& local, blocking_mode mode);

	} // net
	
} // bump
