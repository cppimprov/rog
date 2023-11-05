
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
	auto connection = net::make_tcp_stream(endpoint.m_endpoints.at(0), net::blocking_mode::NON_BLOCKING).unwrap();

	auto retry_timer = timer();
	auto const retry_time = std::chrono::seconds(2);

	// connect to server
	while (true)
	{
		if (connection.is_open())
		{
			auto const& [read, write, except] = connection.select().unwrap();

			if (write)
			{
				std::cout << "connected to server!" << std::endl;
				break;
			}

			if (!connection.is_open())
				retry_timer = timer();
		}
		else
		{
			if (retry_timer.get_elapsed_time() >= retry_time)
				connection = net::make_tcp_stream(endpoint.m_endpoints.at(0), net::blocking_mode::NON_BLOCKING).unwrap();
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
		{
			std::cout << "server disconnected!" << std::endl;
			break;
		}

		// send a message every x seconds
		if (msg_timer.get_elapsed_time() >= msg_time)
		{
			std::cout << "sending message!" << std::endl;

			auto const msg = std::string("msg: ") + std::to_string(msg_num++);
			msg_buffer.push_back(msg.begin(), msg.end());

			while (!msg_buffer.empty())
			{
				auto const bytes_sent = msg_buffer.send(connection);

				if (bytes_sent.has_error())
				{
					if (bytes_sent.error().code() == std::errc::resource_unavailable_try_again ||
					    bytes_sent.error().code() == std::errc::operation_would_block)
						continue;

					std::cout << "send error: " << bytes_sent.error().what() << std::endl;
					connection.close();
					break;
				}
			}

			msg_timer = timer();
		}

		// receive messages
		{
			auto const bytes = connection.receive(std::span<std::uint8_t>(read_buffer.data(), read_buffer.size()));

			if (bytes.has_error())
			{
				if (bytes.error().code() == std::errc::resource_unavailable_try_again ||
				    bytes.error().code() == std::errc::operation_would_block)
					continue;

				std::cout << "receive error: " << bytes.error().what() << std::endl;
				connection.close();
				continue;
			}

			if (bytes.value() == 0)
			{
				std::cout << "server disconnected!" << std::endl;
				connection.close();
				break;
			}

			std::cout << "message received: " << std::string(read_buffer.begin(), read_buffer.begin() + bytes.value());
		}
	}

	std::cout << "done!" << std::endl;
}
