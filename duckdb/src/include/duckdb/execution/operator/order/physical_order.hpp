//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/order/physical_order.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/parallel/pipeline.hpp"
#include "duckdb/planner/bound_query_node.hpp"

namespace duckdb {

class OrderGlobalSinkState;

} // namespace duckdb
