//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/planner/logical_operator_visitor.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/planner/bound_tokens.hpp"
#include "duckdb/planner/logical_tokens.hpp"

#include <functional>

namespace duckdb {
//! The LogicalOperatorVisitor is an abstract base class that implements the
//! Visitor pattern on LogicalOperator.
class LogicalOperatorVisitor {
public:
	virtual ~LogicalOperatorVisitor() {
	}

	static void EnumerateExpressions(LogicalOperator &op,
	                                 const std::function<void(unique_ptr<Expression> *child)> &callback);
};
} // namespace duckdb
