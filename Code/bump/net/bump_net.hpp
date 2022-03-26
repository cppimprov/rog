#pragma once

#include "bump_net_address_lookup.hpp"
#include "bump_net_context.hpp"
#include "bump_net_endpoint.hpp"
#include "bump_net_ip_address.hpp"
#include "bump_net_ip_enums.hpp"
#include "bump_net_tcp_connection.hpp"
#include "bump_net_tcp_listener.hpp"
#include "bump_net_tcp_requester.hpp"
#include "bump_net_udp_connection.hpp"
#include "bump_net_platform.hpp"
#include "bump_net_send_buffer.hpp"


// todo:

	// udp_server_test project
	// udp_client_test project

	// then make the actual server!

	// test tcp_listener code
	// test udp stuff

// todo (sometime):

	// socket options (e.g. getting SO_ERROR)?
	// setters for endpoint?
	// set endpoint ports directly in get_address_info, instead of converting to string and back?
	// set address and port directly in `get_endpoint`?
	// unit tests...
