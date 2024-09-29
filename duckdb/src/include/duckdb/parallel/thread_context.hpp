//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parallel/thread_context.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/main/query_profiler.hpp"

namespace duckdb {
class ClientContext;

//! The ThreadContext holds thread-local info for parallel usage
class ThreadContext {
public:
	explicit ThreadContext(ClientContext &context);
};

} // namespace duckdb
