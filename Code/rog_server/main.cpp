
#include <bump_log.hpp>
#include <bump_net.hpp>

#include <iostream>
#include <span>
#include <string>

// static void print_endpoint(bump::net::ip::endpoint const& endpoint)
// {
// 	namespace net = bump::net;
// 	namespace ip = bump::net::ip;

// 	using std::to_string;

// 	auto const node = ip::get_name_info(endpoint);
// 	auto const family = endpoint.get_address_family();
// 	auto const address = to_string(endpoint.get_address());
// 	auto const port = to_string(endpoint.get_port());

// 	if (node.has_value()) std::cout << "name: " << node.value() << " ";
// 	std::cout << "family: " << (family == ip::address_family::V4 ? "v4" : "v6") << " ";
// 	std::cout << "ip: " << address << " port: " << port << "\n";

// 	// if (family == ip::address_family::V6)
// 	// {
// 	// 	auto const& sa6 = reinterpret_cast<sockaddr_in6 const&>(endpoint.get_address_storage());
// 	// 	std::cout << "scope: " << sa6.sin6_scope_id << "\n";
// 	// }
// }

// static void print_address_info(bump::net::ip::address_info const& info)
// {
// 	namespace net = bump::net;
// 	namespace ip = bump::net::ip;

// 	std::cout << "address_info: \n";
// 	std::cout << "node name: " << info.m_node_name << "\n";
// 	std::cout << "canonical name: " << info.m_canonical_name << "\n";
// 	std::cout << "endpoints: " << info.m_endpoints.size() << "\n";

// 	auto i = 0;

// 	for (auto const& endpoint : info.m_endpoints)
// 	{
// 		std::cout << "[" << i++ << "] ";
// 		print_endpoint(endpoint);
// 	}
// }

// int main()
// {
// 	namespace net = bump::net;
// 	namespace ip = bump::net::ip;

// 	auto context = net::init_context().unwrap();

// 	{
// 		auto any = ip::get_address_info_any(ip::address_family::UNSPECIFIED, ip::protocol::TCP, 0);
// 		print_address_info(any.value());

// 		auto loop = ip::get_address_info_loopback(ip::address_family::UNSPECIFIED, ip::protocol::TCP, 0);
// 		print_address_info(loop.value());
// 	}
	
// 	{
// 		auto a = ip::address(0x2606,0x2800,0x220,0x1,0x248,0x1893,0x25c8,0x1946);
// 		std::cout << ip::to_string(a) << std::endl;

// 		auto b = ip::try_parse_address("192.168.1.1").value();
// 		std::cout << ip::to_string(b) << std::endl;
// 	}

// 	{
// 		auto info1 = ip::get_endpoint(
// 			ip::protocol::TCP, 
// 			ip::address{ 93, 184, 216, 34 }, 80);

// 		print_endpoint(info1.value());
		
// 		auto info2 = ip::get_address_info(
// 			ip::address_family::V4,
// 			ip::protocol::TCP,
// 			"www.example.com", 80, true);
			
// 		print_address_info(info2.value());

// 		std::cout << (info1.value() == info2.value().m_endpoints.at(0)) << std::endl;
// 	}
	
// 	{
// 		auto info = ip::get_address_info(
// 			ip::address_family::UNSPECIFIED,
// 			ip::protocol::TCP,
// 			"www.example.com", 80, true);
			
// 		print_address_info(info.value());
// 	}

// 	{
// 		auto a = ip::address(192, 168, 1, 1);
// 		auto b = a;
// 		auto c = ip::address(192, 168, 1, 2);

// 		std::cout << (a == b) << " " << (a == c) << std::endl;
// 	}
	

// 	{
// 		auto a = ip::address(0x2606,0x2800,0x220,0x1,0x248,0x1893,0x25c8,0x1946);
// 		auto b = a;
// 		auto c = ip::address(192, 168, 1, 2);

// 		std::cout << (a == b) << " " << (a == c) << std::endl;
// 	}
		
// 	{
// 		auto a = ip::address(0x2606,0x2800,0x220,0x1,0x248,0x1893,0x25c8,0x1946);
// 		auto b = a;
// 		auto c = ip::address(0x2606,0x2800,0x220,0x2,0x248,0x1893,0x25c8,0x1946);

// 		std::cout << (a == b) << " " << (a == c) << std::endl;
// 	}

// 	std::cout << ip::get_port("https", ip::protocol::TCP).unwrap() << std::endl;
// 	std::cout << ip::get_service_name(443, ip::protocol::TCP).unwrap() << std::endl;

// 	bump::log_info("done!");
// }


int main()
{
	using namespace bump;
	namespace ip = bump::net::ip;

	auto context = net::init_context().unwrap();

	auto connection = ip::tcp_connection();

	// connect:
	{
		auto info = ip::get_address_info(
			ip::address_family::UNSPECIFIED, ip::protocol::TCP,
			"www.google.com", 80);
		
		die_if(!info.has_value());

		auto requester = ip::connect(info.value().m_endpoints.at(0), net::blocking_mode::NON_BLOCKING).unwrap();

		while (true)
		{
			connection = requester.check().unwrap();

			if (connection.is_open())
				break;
		}

		die_if(!connection.is_open());

		std::cout << "connected" << std::endl;
	}

	// request:
	{
		auto request = std::string("GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n");

		auto buffer = net::send_buffer(1024);
		buffer.push_back(request.begin(), request.end());

		auto bytes_sent = std::size_t{ 0 };

		while (!buffer.empty())
			bytes_sent += buffer.send(connection).unwrap();

		die_if(bytes_sent != request.size());

		std::cout << "request sent" << std::endl;

		auto answer = std::string("\0", 1024);
		auto answer_length = std::size_t{ 0 };

		while (connection.is_open())
		{
			auto const received = connection.receive(std::span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(answer.data()), answer.size())).unwrap();
			answer_length += received;

			if (received != 0)
				std::cout << std::string(answer.begin(), answer.begin() + received);
		}

		std::cout << "\n\nreceived " << answer_length << " bytes.\n";
	}

	std::cout << "done!" << std::endl;
}
