#include "bump_net_socket.hpp"

#include "bump_narrow_cast.hpp"
#include "bump_net_endpoint.hpp"

#include <utility>

namespace bump
{
	
	namespace net
	{

		namespace platform
		{

			result<socket, std::system_error> open_socket(ip_address_family address_family, ip_protocol protocol)
			{
				auto const result = ::socket(get_ai_family(address_family), get_ai_socktype(protocol), get_ai_protocol(protocol));

				if (result == invalid_socket_handle)
					return make_err(get_last_error());
				
				return make_ok(socket(result));
			}

			result<void, std::system_error> close_socket(socket& s)
			{
#if defined(BUMP_NET_WS2)
				auto const result = ::closesocket(s.release());
#else
				auto const result = ::close(s.release());
#endif

				if (result != 0)
					return make_err(get_last_error());
				
				return make_ok();
			}

			result<void, std::system_error> set_blocking_mode(socket const& s, blocking_mode mode)
			{
#if defined(BUMP_NET_WS2)
				auto arg = (mode == blocking_mode::NON_BLOCKING ? 1ul : 0ul);
				auto const result = ::ioctlsocket(s.get_handle(), FIONBIO, &arg);
#else
				auto arg = (mode == blocking_mode::NON_BLOCKING ? 1 : 0);
				auto const result = ::ioctl(s.get_handle(), FIONBIO, &arg);
#endif

				if (result != 0)
					return make_err(get_last_error());
				
				return make_ok();
			}

			result<void, std::system_error> bind(socket const& s, endpoint const& endpoint)
			{
				auto const result = ::bind(s.get_handle(), reinterpret_cast<::sockaddr const*>(&endpoint.get_address_storage()), endpoint.get_address_length());

				if (result != 0)
					return make_err(get_last_error());
				
				return make_ok();
			}

			result<void, std::system_error> connect(socket const& s, endpoint const& endpoint)
			{
				auto const result = ::connect(s.get_handle(), reinterpret_cast<::sockaddr const*>(&endpoint.get_address_storage()), endpoint.get_address_length());

				if (result != 0)
					return make_err(get_last_error());

				return make_ok();
			}

			result<void, std::system_error> listen(socket const& s)
			{
				auto const result = ::listen(s.get_handle(), SOMAXCONN);

				if (result != 0)
					return make_err(get_last_error());
				
				return make_ok();
			}

			result<std::tuple<socket, endpoint>, std::system_error> accept(socket const& s)
			{
				auto addr = ::sockaddr_storage();
				auto addr_len = socklen_t{ sizeof(sockaddr_storage) };
				auto const result = ::accept(s.get_handle(), reinterpret_cast<::sockaddr*>(&addr), &addr_len);

				if (result == invalid_socket_handle)
					return make_err(get_last_error());

				return make_ok(std::make_tuple(socket(result), net::endpoint(addr, addr_len)));
			}

			result<select_result, std::system_error> select(socket const& s)
			{
				auto read_fds = fd_set();
				FD_ZERO(&read_fds);
				FD_SET(s.get_handle(), &read_fds);
				
				auto write_fds = fd_set();
				FD_ZERO(&write_fds);
				FD_SET(s.get_handle(), &write_fds);

				auto except_fds = fd_set();
				FD_ZERO(&except_fds);
				FD_SET(s.get_handle(), &except_fds);

				auto timeout = timeval{ 0, 0 };
				auto n_fds = static_cast<int>(s.get_handle()) + 1;

				auto const result = ::select(n_fds, &read_fds, &write_fds, &except_fds, &timeout);

				if (result == socket_error)
					return make_err(get_last_error());

				if (result == 0)
					return make_ok(select_result{ false, false, false });

				return make_ok(select_result
				{ 
					static_cast<bool>(FD_ISSET(s.get_handle(), &read_fds)),
					static_cast<bool>(FD_ISSET(s.get_handle(), &write_fds)),
					static_cast<bool>(FD_ISSET(s.get_handle(), &except_fds))
				});
			}

			result<std::size_t, std::system_error> send(socket const& s, std::span<const std::uint8_t> data)
			{
				auto const result = ::send(s.get_handle(), reinterpret_cast<char const*>(data.data()), static_cast<int>(data.size()), 0);

				if (result == socket_error)
					return make_err(get_last_error());

				return make_ok(narrow_cast<std::size_t>(result));
			}

			result<std::size_t, std::system_error> send_to(socket const& s, endpoint const& remote, std::span<const std::uint8_t> data)
			{
				auto const result = ::sendto(s.get_handle(), reinterpret_cast<char const*>(data.data()), static_cast<int>(data.size()), 0, reinterpret_cast<::sockaddr const*>(&remote.get_address_storage()), remote.get_address_length());

				if (result == socket_error)
					return make_err(get_last_error());

				return make_ok(narrow_cast<std::size_t>(result));
			}

			result<std::size_t, std::system_error> receive(socket const& s, std::span<std::uint8_t> buffer)
			{

				// note:
				// on Windows we get WSAEMSGSIZE if the buffer is too small
				// on Linux, we pass MSG_TRUNC, so recvfrom will return the actual size of the packet
				// we can then check if the packet has been truncated, and manually return an error

#if defined(BUMP_NET_WS2)
				auto flags = 0;
#else
				auto flags = MSG_TRUNC;
#endif

				auto const result = ::recv(s.get_handle(), reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), flags);

				if (result == socket_error)
					return make_err(get_last_error());

				if (result > buffer.size())
					return make_err(std::system_error(make_error_code(std::errc::message_size), "received packet was truncated!"));

				return make_ok(narrow_cast<std::size_t>(result));
			}
			
			result<std::tuple<endpoint, std::size_t>, std::system_error> receive_from(socket const& s, std::span<std::uint8_t> buffer)
			{

				// note:
				// on Windows we get WSAEMSGSIZE if the buffer is too small
				// on Linux, we pass MSG_TRUNC, so recvfrom will return the actual size of the packet
				// we can then check if the packet has been truncated, and manually return an error

#if defined(BUMP_NET_WS2)
				auto flags = 0;
#else
				auto flags = MSG_TRUNC;
#endif

				auto addr = ::sockaddr_storage();
				auto addr_len = socklen_t{ sizeof(::sockaddr_storage) };
				auto const result = ::recvfrom(s.get_handle(), reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), flags, reinterpret_cast<sockaddr*>(&addr), &addr_len);

				if (result == socket_error)
					return make_err(get_last_error());

				if (result > buffer.size())
					return make_err(std::system_error(make_error_code(std::errc::message_size), "received packet was truncated!"));

				return make_ok(std::tuple<endpoint, std::size_t>{ endpoint(addr, addr_len), narrow_cast<std::size_t>(result) });
			}

		} // platform
		
		socket::socket():
			m_handle(platform::invalid_socket_handle) { }
		
		socket::socket(platform::socket_handle handle):
			m_handle(handle) { }
		
		socket::socket(socket&& other):
			socket()
		{
			using std::swap;
			swap(m_handle, other.m_handle);
		}

		socket& socket::operator=(socket&& other)
		{
			auto temp = socket(std::move(other));

			using std::swap;
			swap(m_handle, temp.m_handle);

			return *this;
		}

		socket::~socket()
		{
			close();
		}

		bool socket::is_open() const
		{
			return (m_handle != platform::invalid_socket_handle);
		}

		result<void, std::system_error> socket::close()
		{
			if (!is_open())
				return make_ok();
				
			return platform::close_socket(*this);
		}

		platform::socket_handle socket::get_handle() const
		{
			return m_handle;
		}

		platform::socket_handle socket::release()
		{
			auto temp = m_handle;
			m_handle = platform::invalid_socket_handle;
			return temp;
		}
		
		result<socket, std::system_error> make_socket(ip_address_family address_family, ip_protocol protocol)
		{
			return platform::open_socket(address_family, protocol);
		}

		result<void, std::system_error> socket::set_blocking_mode(blocking_mode mode) const
		{
			return platform::set_blocking_mode(*this, mode);
		}

		result<void, std::system_error> socket::bind(endpoint const& local) const
		{
			return platform::bind(*this, local);
		}

		result<void, std::system_error> socket::listen() const
		{
			return platform::listen(*this);
		}

		result<void, std::system_error> socket::connect(endpoint const& remote) const
		{
			return platform::connect(*this, remote);
		}
		
		result<std::tuple<socket, endpoint>, std::system_error> socket::accept() const
		{
			return platform::accept(*this);
		}

		result<select_result, std::system_error> socket::select() const
		{
			return platform::select(*this);
		}

		result<std::size_t, std::system_error> socket::send(std::span<const std::uint8_t> data) const
		{
			return platform::send(*this, data);
		}

		result<std::size_t, std::system_error> socket::send_to(endpoint const& remote, std::span<const std::uint8_t> data) const
		{
			return platform::send_to(*this, remote, data);
		}

		result<std::size_t, std::system_error> socket::receive(std::span<std::uint8_t> buffer) const
		{
			return platform::receive(*this, buffer);
		}

		result<std::tuple<endpoint, std::size_t>, std::system_error> socket::receive_from(std::span<std::uint8_t> buffer) const
		{
			return platform::receive_from(*this, buffer);
		}

		result<socket, std::system_error> make_tcp_stream(endpoint const& remote, blocking_mode mode)
		{
			auto socket_result = make_socket(remote.get_address_family(), ip_protocol::TCP);

			if (!socket_result.has_value())
				return make_err(socket_result.error());
			
			auto socket = socket_result.unwrap();

			auto blocking_mode_result = socket.set_blocking_mode(mode);

			if (!blocking_mode_result)
				return make_err(blocking_mode_result.error());

			auto connect_result = socket.connect(remote);

			if (connect_result.has_error() && 
			    connect_result.error().code() != std::errc::operation_would_block &&
			    connect_result.error().code() != std::errc::resource_unavailable_try_again)
				return make_err(connect_result.error());

			return make_ok(std::move(socket));
		}
		
		result<socket, std::system_error> make_tcp_listener(endpoint const& local, blocking_mode mode)
		{
			auto socket_result = make_socket(local.get_address_family(), ip_protocol::TCP);

			if (!socket_result.has_value())
				return make_err(socket_result.error());

			auto socket = socket_result.unwrap();
			
			auto blocking_mode_result = socket.set_blocking_mode(mode);

			if (!blocking_mode_result)
				return make_err(blocking_mode_result.error());
		
			auto bind_result = socket.bind(local);

			if (!bind_result.has_value())
				return make_err(bind_result.error());
			
			auto listen_result = socket.listen();

			if (!listen_result.has_value())
				return make_err(listen_result.error());
			
			return make_ok(std::move(socket));
		}

		result<socket, std::system_error> make_udp_socket(endpoint const& local, blocking_mode mode)
		{
			auto socket_result = make_socket(local.get_address_family(), ip_protocol::UDP);

			if (!socket_result.has_value())
				return make_err(socket_result.error());
			
			auto socket = socket_result.unwrap();

			auto blocking_mode_result = socket.set_blocking_mode(mode);

			if (!blocking_mode_result.has_value())
				return make_err(blocking_mode_result.error());
			
			auto bind_result = socket.bind(local);

			if (!bind_result.has_value())
				return make_err(bind_result.error());
			
			return make_ok(std::move(socket));
		}

	} // net
	
} // bump
