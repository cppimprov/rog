#pragma once

#include "bump_net_error.hpp"
#include "bump_result.hpp"

#include <cstdint>

namespace bump
{
	
	namespace net
	{
		
		class context
		{
		public:

			context(context const&) = delete;
			context& operator=(context const&) = delete;

			context(context&& other);
			context& operator=(context&& other);
			~context();

			bool is_active() const;
			result<void, error> shutdown();

		private:

			explicit context(bool active);
			friend result<context, error> init_context();

			bool m_active;
		};
		
		result<context, error> init_context();
		
	} // net
	
} // bump
