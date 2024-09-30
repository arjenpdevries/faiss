//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/aggregate/physical_ungrouped_aggregate.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/unordered_map.hpp"
#include "duckdb/execution/operator/aggregate/distinct_aggregate_data.hpp"
#include "duckdb/execution/operator/aggregate/grouped_aggregate_data.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/execution/radix_partitioned_hashtable.hpp"
#include "duckdb/parser/group_by_node.hpp"
#include "duckdb/planner/expression.hpp"

namespace duckdb {} // namespace duckdb
