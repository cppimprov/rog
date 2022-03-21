#pragma once

#include "bump_net_ip_enums.hpp"
#include "bump_net_platform.hpp"
#include "bump_result.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <system_error>

namespace bump
{
	
	namespace net
	{

		namespace ip { class endpoint; }

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

			bool is_open() const;
			result<void, std::system_error> close();

			platform::socket_handle get_handle() const;
			platform::socket_handle release();

		private:

			platform::socket_handle m_handle;
		};

		result<socket, std::system_error> open_socket(ip::address_family address_family, ip::protocol protocol);

		// todo: should these be member functions???
		result<void, std::system_error> set_blocking_mode(socket const& socket, blocking_mode mode);
		result<void, std::system_error> bind(socket const& socket, ip::endpoint const& endpoint);
		result<void, std::system_error> connect(socket const& socket, ip::endpoint const& endpoint);
		result<void, std::system_error> listen(socket const& socket);
		result<socket, std::system_error> accept(socket const& socket);
		result<std::optional<bool>, std::system_error> check(socket const& socket);
		result<std::size_t, std::system_error> send(socket const& socket, std::span<const std::uint8_t> data);
		result<std::optional<std::size_t>, std::system_error> receive(socket const& socket, std::span<std::uint8_t> buffer);
		
	} // net
	
} // bump
