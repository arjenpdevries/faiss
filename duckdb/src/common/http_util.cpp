#include "duckdb/common/http_util.hpp"

#include "duckdb/common/operator/cast_operators.hpp"
#include "duckdb/common/string_util.hpp"

namespace duckdb {

void HTTPUtil::ParseHTTPProxyHost(string &proxy_value, string &hostname_out, idx_t &port_out, idx_t default_port) {
	auto proxy_split = StringUtil::Split(proxy_value, ":");
	if (proxy_split.size() == 1) {
		hostname_out = proxy_split[0];
		port_out = default_port;
	} else if (proxy_split.size() == 2) {
		idx_t port;
		hostname_out = proxy_split[0];
		port_out = port;
	} else {
		throw InvalidInputException("Failed to parse http_proxy '%s' into a host and port", proxy_value);
	}
}

} // namespace duckdb
