
#include <bump_log.hpp>
#include <bump_net.hpp>

int main()
{
	auto context = bump::net::init_context(); // kinda awkward... unwrap()?

	if (!context.has_value())
	{
		bump::log_error("nope!");
		bump::die();
	}

	bump::log_info("done!");
}
