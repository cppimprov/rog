#include "bump_net_context.hpp"

#include <utility>

namespace bump
{
	
	namespace net
	{

		namespace platform
		{

			// todo: move these to "bump_net_platform.cpp"
			
			result<void, error> init_socket_library()
			{
				return make_ok(); // todo: implement me!
			}

			result<void, error> shutdown_socket_library()
			{
				return make_ok(); // todo: implement me!
			}
			
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

		result<void, error> context::shutdown()
		{
			if (m_active)
			{
				m_active = false;
				return platform::shutdown_socket_library();
			}
			
			return make_ok();
		}

		result<context, error> init_context()
		{
			auto res = platform::init_socket_library();

			if (!res.has_value())
				return make_err(res.error());

			return make_ok(context(true));
		}

	} // net
	
} // bump
