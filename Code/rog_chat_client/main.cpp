
#include <bump_log.hpp>
#include <bump_net.hpp>
#include <bump_timer.hpp>

#include <chrono>
#include <iostream>

int main()
{
	using namespace bump;

	auto const context = net::init_context().unwrap();
	auto const endpoint = net::get_address_info_loopback(net::ip_address_family::V6, net::ip_protocol::TCP, 4376).unwrap();
	auto connector = net::make_tcp_connector_socket(endpoint.m_endpoints.at(0), net::blocking_mode::NON_BLOCKING).unwrap();

	auto connection = net::socket();

	auto retry_timer = timer();
	auto const retry_time = std::chrono::seconds(2);

	while (true)
	{

		if (connector.is_open())
		{
			if (connector.check())
			{
				std::cout << "connected to server!" << std::endl;
				connection = std::move(connector);
				break;
			}

			if (!connector.is_open())
				retry_timer = timer();
		}
		else
		{
			if (retry_timer.get_elapsed_time() >= retry_time)
				connector = net::make_tcp_connector_socket(endpoint.m_endpoints.at(0), net::blocking_mode::NON_BLOCKING).unwrap();
		}
	}

	die_if(!connection.is_open());

	auto msg_buffer = net::send_buffer(1024);

	auto msg_num = 0;
	auto msg_timer = timer();
	auto msg_time = std::chrono::seconds(2);

	auto read_buffer = std::vector<std::uint8_t>(4096, '\0');

	while (true)
	{
		// disconnected
		if (!connection.is_open())
			break;

		// send a message every x seconds
		if (msg_timer.get_elapsed_time() >= msg_time)
		{
			auto const msg = std::string("msg: ") + std::to_string(msg_num++);
			msg_buffer.push_back(msg.begin(), msg.end());

			while (!msg_buffer.empty())
				msg_buffer.send(connection).unwrap();

			msg_timer = timer();
		}

		// receive messages
		{
			auto const bytes = connection.receive(std::span<std::uint8_t>(read_buffer.data(), read_buffer.size()));

			if (bytes.has_value() && bytes.value() != 0)
				std::cout << std::string(read_buffer.begin(), read_buffer.begin() + bytes.value());
		}
	}

	std::cout << "done!" << std::endl;
}
