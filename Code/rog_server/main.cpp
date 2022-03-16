
#include <bump_log.hpp>
#include <bump_net.hpp>

#include <string>

int main()
{
	auto context = bump::net::init_context().unwrap();

	auto loop = bump::net::ip::get_address("www.google.com", bump::net::ip::name_type::ANY, bump::net::ip::address_family::V4, bump::net::ip::protocol::TCP);

	if (!loop.has_value())
		bump::log_error("nope!");
	else
		bump::log_info(bump::net::ip::get_node_ip(loop.value().at(0)).unwrap());

	bump::log_info("done!");
}
