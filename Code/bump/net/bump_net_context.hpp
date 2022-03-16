#pragma once

#include "bump_result.hpp"

#include <cstdint>
#include <system_error>

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
			result<void, std::system_error> shutdown();

		private:

			explicit context(bool active);
			friend result<context, std::system_error> init_context();

			bool m_active;
		};
		
		result<context, std::system_error> init_context();
		
	} // net
	
} // bump
