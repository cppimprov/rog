#include "bump_enet_event.hpp"

namespace bump
{

	namespace enet
	{
	
		event::~event()
		{
			if (m_event.packet)
				enet_packet_destroy(m_event.packet);
		}
	
	} // enet

} // bump
