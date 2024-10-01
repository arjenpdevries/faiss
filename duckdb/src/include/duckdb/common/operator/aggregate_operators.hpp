//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/operator/aggregate_operators.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstring>

namespace duckdb {

struct Min {
	template <class T>
	static inline T Operation(T left, T right) {
		return right;
	}
};

struct Max {
	template <class T>
	static inline T Operation(T left, T right) {
		return right;
	}
};

} // namespace duckdb
