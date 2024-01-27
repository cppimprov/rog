#pragma once

namespace bump
{

	namespace enet
	{
	
		class context
		{
		public:

			context();
			explicit context(bool was_init);
			~context();

			context(context&&);
			context& operator=(context&&);

			context(const context&) = delete;
			context& operator=(const context&) = delete;

			bool is_active() const;
		
		private:

			bool m_active;
		};

		context initialize();
	
	} // enet

} // bump