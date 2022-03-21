#include "bump_net_socket.hpp"

#include "bump_narrow_cast.hpp"
#include "bump_net_ip_endpoint.hpp"

#include <utility>

namespace bump
{
	
	namespace net
	{

		namespace platform
		{
			
			result<socket_handle, std::system_error> open_socket(ip::address_family address_family, ip::protocol protocol)
			{
				auto const result = ::socket(get_ai_family(address_family), get_ai_socktype(protocol), get_ai_protocol(protocol));

				if (result == invalid_socket_handle)
					return make_err(get_last_error());
				
				return make_ok(result);
			}

			result<void, std::system_error> close_socket(socket& socket)
			{
				auto const result = ::closesocket(socket.release());

				if (result == SOCKET_ERROR)
					return make_err(get_last_error());
				
				return make_ok();
			}

			result<void, std::system_error> set_blocking_mode(socket const& socket, blocking_mode mode)
			{
				auto arg = (mode == blocking_mode::NON_BLOCKING ? 1ul : 0ul);
				auto const result = ::ioctlsocket(socket.get_handle(), FIONBIO, &arg);

				if (result != 0)
					return make_err(get_last_error());
				
				return make_ok();
			}

			result<void, std::system_error> bind(socket const& socket, ip::endpoint const& endpoint)
			{
				auto const result = ::bind(socket.get_handle(), reinterpret_cast<::sockaddr const*>(&endpoint.get_address_storage()), endpoint.get_address_length());

				if (result != 0)
					return make_err(get_last_error());
				
				return make_ok();
			}

			result<void, std::system_error> connect(socket const& socket, ip::endpoint const& endpoint)
			{
				auto const result = ::connect(socket.get_handle(), reinterpret_cast<::sockaddr const*>(&endpoint.get_address_storage()), endpoint.get_address_length());

				if (result != 0)
				{
					auto const err = get_last_error();
					auto const err_value = err.code().value();

#if defined(BUMP_NET_WS2)
					if (err_value == WSAEWOULDBLOCK || err_value == WSAECONNRESET)
						return make_ok();
#else
					if (err_value == EAGAIN || err_value == EINPROGRESS)
						return make_ok();
#endif

					return make_err(err);
				}

				return make_ok();
			}

			result<void, std::system_error> listen(socket const& socket)
			{
				auto const result = ::listen(socket.get_handle(), SOMAXCONN);

				if (result != 0)
					return make_err(get_last_error());
				
				return make_ok();
			}

			result<socket, std::system_error> accept(socket const& socket)
			{
				auto const result = ::accept(socket.get_handle(), nullptr, nullptr);

				if (result == invalid_socket_handle)
				{
					auto const err = get_last_error();
					auto const err_value = err.code().value();

#if defined(BUMP_NET_WS2)
					if (err_value == WSAEWOULDBLOCK || err_value == WSAECONNRESET)
						return make_ok(net::socket());
#else
					if (err_value == EAGAIN || err_value == EWOULDBLOCK || err_value == ECONNABORTED)
						return make_ok(net::socket());
#endif

					return make_err(err);
				}

				return make_ok(net::socket(result));
			}

			result<std::optional<bool>, std::system_error> check(socket const& socket)
			{
				// select call setup...

				auto const result = ::select(0, nullptr, &writefds, &exceptfds, &timeout);

				if (result == SOCKET_ERROR)
					return make_err(get_last_error());

				if (result == 0) // no sockets ready (still connecting)
					return make_ok(std::optional<bool>(std::nullopt));

				// ... failed, return std::optional<bool>(false);

				return make_ok(std::optional<bool>(true));
			}

			result<std::size_t, std::system_error> send(socket const& socket, std::uint8_t const* data, std::size_t data_size)
			{
				auto const result = ::send(socket.get_handle(), reinterpret_cast<char const*>(data), data_size, 0);

				if (result == SOCKET_ERROR)
				{
					auto const err = get_last_error();
					auto const err_value = err.code().value();

#if defined(BUMP_NET_WS2)
					if (err_value == WSAEWOULDBLOCK)
						return make_ok(std::size_t{ 0 });
#else
					if (err_value == EAGAIN || err_value == EWOULDBLOCK)
						return make_ok(std::size_t{ 0 });
#endif

					return make_err(err);
				}

				return make_ok(narrow_cast<std::size_t>(result));
			}

			result<std::optional<std::size_t>, std::system_error> receive(socket const& socket, std::uint8_t* data, std::size_t data_size)
			{
				auto const result = ::recv(socket.get_handle(), reinterpret_cast<char*>(data), data_size, 0);

				if (result == 0 && data_size != 0)
					return make_ok(std::optional<std::size_t>(std::nullopt)); // connection closed!

				if (result == SOCKET_ERROR)
				{
					auto const err = get_last_error();
					auto const err_value = err.code().value();

#if defined(BUMP_NET_WS2)
					if (err_value == WSAEWOULDBLOCK)
						return make_ok(std::optional<std::size_t>(std::size_t{ 0 }));
#else
					if (err_value == EAGAIN || err_value == EWOULDBLOCK)
						return make_ok(std::optional<std::size_t>(std::size_t{ 0 }));
#endif

					return make_err(err);
				}

				return make_ok(std::optional<std::size_t>(narrow_cast<std::size_t>(result)));
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
			swap(m_handle, other.m_handle);

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
		
		result<socket, std::system_error> open_socket(ip::address_family address_family, ip::protocol protocol)
		{
			auto const result = platform::open_socket(address_family, protocol);

			if (!result.has_value())
				return make_err(result.error());
			
			return make_ok(socket(result.value()));
		}
		
		result<void, std::system_error> set_blocking_mode(socket const& socket, blocking_mode mode)
		{
			return platform::set_blocking_mode(socket, mode);
		}

		result<void, std::system_error> bind(socket const& socket, ip::endpoint const& endpoint)
		{
			return platform::bind(socket, endpoint);
		}

		result<void, std::system_error> connect(socket const& socket, ip::endpoint const& endpoint)
		{
			return platform::connect(socket, endpoint);
		}

		result<void, std::system_error> listen(socket const& socket)
		{
			return platform::listen(socket);
		}

		result<socket, std::system_error> accept(socket const& socket)
		{
			return platform::accept(socket);
		}

		result<std::optional<bool>, std::system_error> check(socket const& socket)
		{
			return platform::check(socket);
		}

		result<std::size_t, std::system_error> send(socket const& socket, std::uint8_t const* data, std::size_t data_size)
		{
			return platform::send(socket, data, data_size);
		}

		result<std::optional<std::size_t>, std::system_error> receive(socket const& socket, std::uint8_t* data, std::size_t data_size)
		{
			return platform::receive(socket, data, data_size);
		}

	} // net
	
} // bump
