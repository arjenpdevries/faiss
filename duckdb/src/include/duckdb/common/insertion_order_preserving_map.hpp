//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/insertion_order_preserving_map.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/case_insensitive_map.hpp"
#include "duckdb/common/helper.hpp"
#include "duckdb/common/pair.hpp"
#include "duckdb/common/string.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/common/unordered_map.hpp"
#include "duckdb/common/unordered_set.hpp"

namespace duckdb {

template <typename V>
class InsertionOrderPreservingMap {
public:
	typedef vector<pair<string, V>> VECTOR_TYPE; // NOLINT: matching name of std
	typedef string key_type;                     // NOLINT: matching name of std

public:
	InsertionOrderPreservingMap() {
	}

private:
	VECTOR_TYPE map;
	case_insensitive_map_t<idx_t> map_idx;
};

} // namespace duckdb
