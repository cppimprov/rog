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

			// Change the socket blocking mode (the default mode is blocking).
			// Blocking socket operations do not return until they have sent or received
			// some (but not necessarily all) of the specified data.
			// Non-blocking sockets return immediately.
			result<void, std::system_error> set_blocking_mode(blocking_mode mode) const;

			// Associate the socket with local address / port. This must be done
			// before the socket can accept connections.
			result<void, std::system_error> bind(endpoint const& local) const;

			// Use for a connection-oriented protocol (e.g. TCP) to connect to a specific
			// remote endpoint. Can also be used for UDP to specify the default remote
			// endpoint for send operations / receive operations. Note that this will
			// prevent reception of messages from other endpoints.
			result<void, std::system_error> connect(endpoint const& remote) const;

			// Use for a connection-oriented protocol (e.g. TCP) to listen for incoming
			// connections. Note that the socket must be bound to a local endpoint (using bind())
			// before this function is called.
			result<void, std::system_error> listen() const;

			// Use for a connection-oriented protocol (e.g. TCP) to accept an incoming
			// connection. Note that the socket must be listening for incoming connections
			// (using listen()) before this function is called. Returns a new socket
			// representing the connection, and the remote endpoint. May return a
			// closed socket / invalid endpoint if the socket is non-blocking and no
			// connection is available.
			result<std::tuple<socket, endpoint>, std::system_error> accept() const;

			// Use for a connection-oriented protocol (e.g. TCP) to check if a connection
			// has been established (i.e. use this after calling connect()).
			result<bool, std::system_error> check();

			// Use for a connection-oriented protocol (e.g. TCP) to send data to the remote
			// endpoint. Can also be used for UDP to send data if connect() has been called.
			// Returns the number of bytes sent.
			result<std::size_t, std::system_error> send(std::span<const std::uint8_t> data);

			// Send data to a specific remote endpoint for connectionless protocols (e.g. UDP).
			// If connect() has been called for a connectionless protocol, then the remote endpoint
			// is temporarily overridden for this call only. If called for a connection-oriented
			// protocol (e.g. TCP) then the remote endpoint argument is ignored (i.e. send_to is
			// equivalent to send).
			// Returns the number of bytes sent.
			result<std::size_t, std::system_error> send_to(endpoint const& remote, std::span<const std::uint8_t> data);

			// Use for both connection-oriented and connectionless protocols (e.g. TCP and UDP)
			// to receive data from a remote endpoint. If connect() has been called,
			// this function will only receive data from the connected endpoint.
			// For connectionless protocols (e.g. UDP), the socket must be bound to a local
			// endpoint (using bind()) before this function is called.
			// Returns the number of bytes received.
			result<std::size_t, std::system_error> receive(std::span<std::uint8_t> buffer);

			// Use for both connection-oriented and connectionless protocols (e.g. TCP and UDP).
			// Returns the number of bytes received, and the remote endpoint.
			result<std::tuple<endpoint, std::size_t>, std::system_error> receive_from(std::span<std::uint8_t> buffer);

			bool is_open() const;
			result<void, std::system_error> close();

			platform::socket_handle get_handle() const;
			platform::socket_handle release();
			
		private:

			platform::socket_handle m_handle;
		};

		result<socket, std::system_error> make_socket(ip_address_family address_family, ip_protocol protocol);
		result<socket, std::system_error> make_tcp_listener_socket(endpoint const& local, blocking_mode mode);
		result<socket, std::system_error> make_tcp_connector_socket(endpoint const& remote, blocking_mode mode);
		result<socket, std::system_error> make_udp_socket(endpoint const& local, blocking_mode mode);
		result<socket, std::system_error> make_udp_connected_socket(endpoint const& local, endpoint const& remote, blocking_mode mode);

	} // net
	
} // bump
