#pragma once

#include "bump_net_context.hpp"
#include "bump_net_ip.hpp"
#include "bump_net_platform.hpp"
#include "bump_net_send_buffer.hpp"


// todo:

	// remove the ip namespace!
	// test tcp_listener code
	// test udp stuff

// todo (sometime):

	// socket options (e.g. getting SO_ERROR)?
	// return address from accept?
	// setters for endpoint?
	// set endpoint ports directly in get_address_info, instead of converting to string and back?
	// set address and port directly in `get_endpoint`?
	// unit tests...
