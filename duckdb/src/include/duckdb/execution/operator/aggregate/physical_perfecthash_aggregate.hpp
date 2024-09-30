//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/aggregate/physical_perfecthash_aggregate.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/base_aggregate_hashtable.hpp"
#include "duckdb/execution/physical_operator.hpp"

namespace duckdb {
class ClientContext;
class PerfectAggregateHashTable;

} // namespace duckdb
