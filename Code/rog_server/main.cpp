
#include <bump_log.hpp>
#include <bump_net.hpp>

#include <iostream>
#include <string>

int main()
{
	namespace net = bump::net;
	namespace ip = bump::net::ip;

	auto context = net::init_context().unwrap();

	auto info = ip::get_address_info(
		ip::address_family::UNSPECIFIED,
		ip::protocol::TCP,
		"www.example.com", 80);
	
	if (!info.has_value())
		bump::log_error("nope!");
	else
	{
		std::cout << "found " << info.value().size() << " addresses:\n";

		auto i = 0;

		for (auto const& endpoint : info.value())
		{
			using std::to_string;

			auto const node = ip::get_name_info(endpoint);
			auto const address = to_string(endpoint.get_address()).unwrap();
			auto const port = to_string(endpoint.get_port());

			std::cout << i++ << ":\n";
			if (node.has_value()) std::cout << "name: " << node.value() << "\n";
			std::cout << "ip: " << address << " port: " << port << "\n";
			std::cout << "\n";
		}
	}

	bump::log_info("done!");
}
