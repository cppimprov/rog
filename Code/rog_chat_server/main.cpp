
#include <bump_log.hpp>
#include <bump_net.hpp>

#include <iostream>

int main()
{
	using namespace bump;

	auto const context = net::init_context().unwrap();
	auto const endpoint = net::get_address_info_loopback(net::ip_address_family::V6, net::ip_protocol::TCP, 4376).unwrap();
	auto listener = net::listen(endpoint.m_endpoints.at(0), net::blocking_mode::NON_BLOCKING).unwrap();
	auto connections = std::vector<net::tcp_connection>();
	auto read_buffer = std::vector<std::uint8_t>(4096, '\0');

	while (true)
	{

		// check for new clients
		{
			auto c = listener.accept().unwrap();

			if (c.is_open())
			{
				std::cout << "client connected!" << std::endl;
				connections.push_back(std::move(c));
			}
		}
		
		// handle disconnections
		{
			auto const dc = [] (net::tcp_connection const& c)
			{
				if (!c.is_open())
				{
					std::cout << "client disconnected!" << std::endl;
					return true;
				}

				return false;
			};

			connections.erase(
				std::remove_if(connections.begin(), connections.end(), dc),
				connections.end());
		}

		// receive messages
		{
			for (auto& c : connections)
			{
				auto const bytes = c.receive(std::span<std::uint8_t>(read_buffer.data(), read_buffer.size()));

				if (bytes.has_value() && bytes.value() != 0)
					std::cout << std::string(read_buffer.begin(), read_buffer.begin() + bytes.value()) << std::endl;
			}
		}

		// TODO: send messages!
	}

	std::cout << "done!" << std::endl;
}
