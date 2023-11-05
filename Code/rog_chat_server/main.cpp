
#include <bump_log.hpp>
#include <bump_net.hpp>

#include <iostream>

int main()
{
	using namespace bump;

	auto const context = net::init_context().unwrap();
	auto const endpoint = net::get_address_info_loopback(net::ip_address_family::V6, net::ip_protocol::TCP, 4376).unwrap();
	auto listener = net::make_tcp_listener(endpoint.m_endpoints.at(0), net::blocking_mode::NON_BLOCKING).unwrap();
	auto connections = std::vector<std::tuple<net::socket, std::size_t>>();
	auto read_buffer = std::vector<std::uint8_t>(4096, '\0');

	auto next_client_id = std::size_t{ 0 };
	auto messages = std::vector<std::tuple<std::string, std::size_t>>();
	auto const newline = std::string("\n");

	while (true)
	{

		// check for new clients
		{
			auto accept_result = listener.accept();

			if (accept_result.has_error())
			{
				if (accept_result.error().code() != std::errc::resource_unavailable_try_again &&
				    accept_result.error().code() != std::errc::operation_would_block &&
				    accept_result.error().code() != std::errc::connection_aborted)
				{
					std::cout << "accept error: " << accept_result.error().what() << std::endl;
				}
			}
			else
			{
				auto [c, e] = accept_result.unwrap();

				if (c.is_open())
				{
					std::cout << "client connected!" << std::endl;
					connections.push_back({ std::move(c), next_client_id++ });
				}
			}
		}
		
		// handle disconnections
		{
			auto const dc = [] (std::tuple<net::socket, std::size_t> const& c)
			{
				if (!std::get<0>(c).is_open())
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

		messages.clear();

		// receive messages
		{
			for (auto& c : connections)
			{
				auto &[conn, id] = c;

				auto const bytes = conn.receive(std::span<std::uint8_t>(read_buffer.data(), read_buffer.size()));

				if (bytes.has_error())
				{
					if (bytes.error().code() == std::errc::resource_unavailable_try_again ||
					    bytes.error().code() == std::errc::operation_would_block)
						continue;

					std::cout << "receive error: " << bytes.error().what() << std::endl;
					conn.close();
					break;
				}

				if (bytes.value() == 0)
				{
					conn.close();
					continue;
				}

				auto message = std::string(read_buffer.begin(), read_buffer.begin() + bytes.value());
				std::cout << "received message: " << message << std::endl;
				messages.push_back({ std::move(message), id });
			}
		}

		// send messages
		{
			auto msg_buffer = net::send_buffer(1024);

			for (auto const& m : messages)
			{
				auto const& [message, m_id] = m;

				for (auto& c : connections)
				{
					auto &[conn, c_id] = c;

					if (m_id == c_id) // don't send messages back to the sender...
						continue;

					msg_buffer.push_back(message.begin(), message.end());
					msg_buffer.push_back(newline.begin(), newline.end());

					while (!msg_buffer.empty())
					{
						auto const bytes_sent = msg_buffer.send(conn);

						if (bytes_sent.has_error())
						{
							if (bytes_sent.error().code() == std::errc::resource_unavailable_try_again ||
							    bytes_sent.error().code() == std::errc::operation_would_block)
								continue; // busy loop here... kinda dodgy?
							
							std::cout << "send error: " << bytes_sent.error().what() << std::endl;
							conn.close();
							break;
						}
					}
				}
			}
		}
	}

	std::cout << "done!" << std::endl;
}
