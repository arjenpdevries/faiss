//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/persistent/physical_insert.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/index_vector.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/parser/statement/insert_statement.hpp"
#include "duckdb/planner/expression.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"

namespace duckdb {

class InsertLocalState;

} // namespace duckdb
