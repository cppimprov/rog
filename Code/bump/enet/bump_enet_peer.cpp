#include "bump_enet_peer.hpp"

namespace bump
{

	namespace enet
	{
	
		void peer::send(std::uint8_t channel_id, packet packet)
		{
			die_if(!m_peer);

			enet_peer_send(m_peer, channel_id, packet.release());
		}

		packet peer::receive(std::uint8_t& channel_id)
		{
			die_if(!m_peer);

			return packet(enet_peer_receive(m_peer, &channel_id));
		}
	
	} // enet

} // bump
