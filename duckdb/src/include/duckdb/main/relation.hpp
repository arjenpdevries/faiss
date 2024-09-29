//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/main/relation.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/common/enums/join_type.hpp"
#include "duckdb/common/enums/joinref_type.hpp"
#include "duckdb/common/enums/relation_type.hpp"
#include "duckdb/common/helper.hpp"
#include "duckdb/common/named_parameter_map.hpp"
#include "duckdb/common/unique_ptr.hpp"
#include "duckdb/common/vector.hpp"
#include "duckdb/common/winapi.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/client_context_wrapper.hpp"
#include "duckdb/main/external_dependencies.hpp"
#include "duckdb/main/query_result.hpp"
#include "duckdb/parser/column_definition.hpp"
#include "duckdb/parser/parsed_expression.hpp"
#include "duckdb/parser/result_modifier.hpp"
#include "duckdb/parser/statement/explain_statement.hpp"

namespace duckdb {
struct BoundStatement;

class Binder;
class LogicalOperator;
class QueryNode;
class TableRef;

} // namespace duckdb
