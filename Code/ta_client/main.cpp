
#include <bump_log.hpp>
#include <bump_net.hpp>

#include <iostream>

int main()
{
	using namespace bump;

	auto const net_context = net::init_context().unwrap();
	auto const local_endpoint = net::get_address_info_loopback(net::ip_address_family::V6, net::ip_protocol::UDP, 4377).unwrap().m_endpoints.front();
	auto const remote_endpoint = net::get_address_info_loopback(net::ip_address_family::V6, net::ip_protocol::UDP, 4376).unwrap().m_endpoints.front();
	auto udp_socket = net::make_udp_connected_socket(local_endpoint, remote_endpoint, net::blocking_mode::NON_BLOCKING).unwrap();
	auto udp_read_buffer = std::vector<std::uint8_t>(128, '\0');

	while (true)
	{
		// uh... yep!

	}

	std::cout << "done!" << std::endl;
}

// ... 
