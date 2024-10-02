//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/logging/http_logger.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/fstream.hpp"
#include "duckdb/common/mutex.hpp"
#include "duckdb/common/printer.hpp"
#include "duckdb/main/client_context.hpp"

#include <functional>

namespace duckdb {

//! This has to be templated because we have two namespaces:
//! 1. duckdb_httplib
//! 2. duckdb_httplib_openssl
//! These have essentially the same code, but we cannot convert between them
//! We get around that by templating everything, which requires implementing everything in the header
class HTTPLogger {
public:
	explicit HTTPLogger(ClientContext &context_p) : context(context_p) {
	}

public:
	template <class REQUEST, class RESPONSE>
	std::function<void(const REQUEST &, const RESPONSE &)> GetLogger() {
		return [&](const REQUEST &req, const RESPONSE &res) {
			Log(req, res);
		};
	}

private:
	template <class STREAM, class REQUEST, class RESPONSE>
	static inline void TemplatedWriteRequests(STREAM &out, const REQUEST &req, const RESPONSE &res) {
		out << "HTTP Request:\n";
		out << "\t" << req.method << " " << req.path << "\n";
		for (auto &entry : req.headers) {
			out << "\t" << entry.first << ": " << entry.second << "\n";
		}
		out << "\nHTTP Response:\n";
		out << "\t" << res.status << " " << res.reason << " " << req.version << "\n";
		for (auto &entry : res.headers) {
			out << "\t" << entry.first << ": " << entry.second << "\n";
		}
		out << "\n";
	}

	template <class REQUEST, class RESPONSE>
	void Log(const REQUEST &req, const RESPONSE &res) {
	}

private:
	ClientContext &context;
	mutex lock;
};

} // namespace duckdb
