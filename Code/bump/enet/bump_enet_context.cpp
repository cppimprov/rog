#include "bump_enet_context.hpp"

#include "bump_log.hpp"

#include <enet/enet.h>

#include <utility>

namespace bump
{

	namespace enet
	{

		context::context():
			m_active(false) { }
	
		context::context(bool was_init):
			m_active(was_init) { }
		
		context::~context()
		{
			if (m_active)
				enet_deinitialize();
		}

		context::context(context&& other):
			m_active(other.m_active)
		{
			other.m_active = false;
		}

		context& context::operator=(context&& other)
		{
			auto temp = std::move(other);
			std::swap(m_active, temp.m_active);
			return *this;
		}

		bool context::is_active() const
		{
			return m_active;
		}

		context initialize()
		{
			if (enet_initialize() != 0)
			{
				log_error("failed to initialize enet!");
				return { };
			}

			return context(true);
		}
	
	} // enet

} // bump
