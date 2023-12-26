#include "bump_enet_packet.hpp"

#include <utility>

namespace bump
{

	namespace enet
	{

		packet::packet(std::string_view data, int flags):
			m_packet(enet_packet_create(data.data(), data.size(), flags)) { }
	
		packet::packet(std::uint8_t const* data, std::size_t data_size, int flags):
			m_packet(enet_packet_create(data, data_size, flags)) { }
		
		packet::packet(packet&& other):
			m_packet(other.m_packet) { other.m_packet = nullptr; }
		
		packet& packet::operator=(packet&& other)
		{
			auto temp = std::move(other);
			std::swap(m_packet, temp.m_packet);
			return *this;
		}

		packet::~packet()
		{
			enet_packet_destroy(m_packet);
		}
	
	} // enet

} // bump
