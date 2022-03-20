#include "bump_net_context.hpp"

#include "bump_net_platform.hpp"

#include <utility>

namespace bump
{
	
	namespace net
	{

		namespace platform
		{

#if defined(BUMP_NET_WS2)

			result<void, std::system_error> init_socket_library()
			{
				auto data = WSADATA();
				auto const result = ::WSAStartup(MAKEWORD(2, 2), &data);

				if (result != 0)
					return make_err(std::system_error(std::error_code(result, std::system_category())));

				return make_ok();
			}

			result<void, std::system_error> shutdown_socket_library()
			{
				auto const result = ::WSACleanup();

				if (result != 0)
					return make_err(get_last_error());

				return make_ok();
			}

#else

			result<void, std::system_error> init_socket_library()
			{
				return make_ok(); // nothing to do!
			}

			result<void, std::system_error> shutdown_socket_library()
			{
				return make_ok(); // nothing to do!
			}

#endif

		} // platform

		context::context(bool active):
			m_active(active)
		{
			
		}

		context::context(context&& other):
			m_active(other.m_active)
		{
			other.m_active = false;
		}

		context& context::operator=(context&& other)
		{
			auto temp = std::move(other);

			using std::swap;
			swap(m_active, temp.m_active);

			return *this;
		}

		context::~context()
		{
			(void)shutdown();
		}

		bool context::is_active() const
		{
			return m_active;
		}

		result<void, std::system_error> context::shutdown()
		{
			if (m_active)
			{
				m_active = false;
				return platform::shutdown_socket_library();
			}
			
			return make_ok();
		}

		result<context, std::system_error> init_context()
		{
			auto res = platform::init_socket_library();

			if (!res.has_value())
				return make_err(res.error());

			return make_ok(context(true));
		}

	} // net
	
} // bump
