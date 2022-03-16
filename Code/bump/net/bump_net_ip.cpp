#include "bump_net_ip.hpp"

#include "bump_net_platform.hpp"

namespace bump
{
	
	namespace net
	{
		
		namespace ip
		{

			namespace
			{

				std::vector<endpoint> get_endpoints(platform::addrinfo_ptr const& info)
				{
					if (!info) return { };

					auto out = std::vector<endpoint>();

					auto ptr = info.get();

					while (ptr)
					{
						out.emplace_back(*ptr);
						ptr = ptr->ai_next;
					}

					return out;
				}

				std::string get_canonical_name(platform::addrinfo_ptr const& info)
				{
					if (!info) return { };
					auto const ptr = info.get();
					if (!ptr->ai_canonname) return { };
					return std::string(ptr->ai_canonname);
				}

				ip::address_family get_ip_address_family(int family)
				{
					switch (family)
					{
					case AF_INET: return ip::address_family::V4;
					case AF_INET6: return ip::address_family::V6;
					case AF_UNSPEC: return ip::address_family::UNSPECIFIED;
					}

					die();
				}

			} // unnamed

			endpoint::endpoint(::addrinfo const& info):
				m_length(info.ai_addrlen),
				m_address()
			{
				die_if(info.ai_addrlen > sizeof(m_address));
				std::memcpy(&m_address, info.ai_addr, m_length);
			}

			address_family endpoint::get_address_family() const
			{
				return get_ip_address_family(m_address.ss_family);
			}

			result<std::vector<endpoint>, std::system_error> get_wildcard_address(address_family address_family, protocol protocol)
			{
				auto info = platform::get_wildcard_address(address_family, protocol);

				if (!info.has_value())
					return make_err(info.error());

				return make_ok(get_endpoints(info.value()));
			}

			result<std::vector<endpoint>, std::system_error> get_wildcard_address(std::string const& service_name, name_type service_name_type, address_family address_family, protocol protocol)
			{
				auto info = platform::get_wildcard_address(service_name, service_name_type, address_family, protocol);

				if (!info.has_value())
					return make_err(info.error());

				return make_ok(get_endpoints(info.value()));
			}

			result<std::vector<endpoint>, std::system_error> get_loopback_address(address_family address_family, protocol protocol)
			{
				auto info = platform::get_loopback_address(address_family, protocol);

				if (!info.has_value())
					return make_err(info.error());

				return make_ok(get_endpoints(info.value()));
			}

			result<std::vector<endpoint>, std::system_error> get_loopback_address(std::string const& service_name, name_type service_name_type, address_family address_family, protocol protocol)
			{
				auto info = platform::get_loopback_address(service_name, service_name_type, address_family, protocol);

				if (!info.has_value())
					return make_err(info.error());

				return make_ok(get_endpoints(info.value()));
			}

			result<std::vector<endpoint>, std::system_error> get_address(std::string const& node_name, name_type node_name_type, address_family address_family, protocol protocol)
			{
				auto info = platform::get_address(node_name, node_name_type, address_family, protocol);

				if (!info.has_value())
					return make_err(info.error());
				
				return make_ok(get_endpoints(info.value()));
			}

			result<std::vector<endpoint>, std::system_error> get_address(std::string const& node_name, name_type node_name_type, std::string const& service_name, name_type service_name_type, address_family address_family, protocol protocol)
			{
				auto info = platform::get_address(node_name, node_name_type, service_name, service_name_type, address_family, protocol);

				if (!info.has_value())
					return make_err(info.error());

				return make_ok(get_endpoints(info.value()));
			}
			
			result<std::tuple<std::vector<endpoint>, std::string>, std::system_error> get_address_and_canonical_name(std::string const& node_name, name_type node_name_type, address_family address_family, protocol protocol)
			{
				auto info = platform::get_address_and_canonical_name(node_name, node_name_type, address_family, protocol);

				if (!info.has_value())
					return make_err(info.error());

				return make_ok(std::make_tuple(get_endpoints(info.value()), get_canonical_name(info.value())));
			}

			result<std::tuple<std::vector<endpoint>, std::string>, std::system_error> get_address_and_canonical_name(std::string const& node_name, name_type node_name_type, std::string const& service_name, name_type service_name_type, address_family address_family, protocol protocol)
			{
				auto info = platform::get_address_and_canonical_name(node_name, node_name_type, service_name, service_name_type, address_family, protocol);

				if (!info.has_value())
					return make_err(info.error());
				
				return make_ok(std::make_tuple(get_endpoints(info.value()), get_canonical_name(info.value())));
			}

			result<std::string, std::system_error> get_node_name(endpoint const& endpoint, qualify_local_host qualify)
			{
				return platform::get_node_name(endpoint, qualify);
			}

			result<std::string, std::system_error> get_node_name(endpoint const& endpoint)
			{
				return platform::get_node_name(endpoint);
			}

			result<std::string, std::system_error> get_node_ip(endpoint const& endpoint)
			{
				return platform::get_node_ip(endpoint);
			}

			result<std::string, std::system_error> get_port(endpoint const& endpoint)
			{
				return platform::get_port(endpoint);
			}
			
		} // ip
		
	} // net
	
} // bump
