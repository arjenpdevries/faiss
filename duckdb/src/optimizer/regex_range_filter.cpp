#include "duckdb/optimizer/regex_range_filter.hpp"

#include "duckdb/catalog/catalog_entry/scalar_function_catalog_entry.hpp"
#include "duckdb/function/scalar/regexp.hpp"
#include "duckdb/function/scalar/string_functions.hpp"
#include "duckdb/planner/expression.hpp"
#include "duckdb/planner/expression/bound_comparison_expression.hpp"
#include "duckdb/planner/expression/bound_conjunction_expression.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"
#include "duckdb/planner/expression/bound_function_expression.hpp"
#include "duckdb/planner/operator/logical_filter.hpp"

namespace duckdb {

unique_ptr<LogicalOperator> RegexRangeFilter::Rewrite(unique_ptr<LogicalOperator> op) {

	for (idx_t child_idx = 0; child_idx < op->children.size(); child_idx++) {
		op->children[child_idx] = Rewrite(std::move(op->children[child_idx]));
	}

	if (op->type != LogicalOperatorType::LOGICAL_FILTER) {
		return op;
	}

	auto new_filter = make_uniq<LogicalFilter>();

	for (auto &expr : op->expressions) {
		if (expr->type == ExpressionType::BOUND_FUNCTION) {
		}
	}

	if (!new_filter->expressions.empty()) {
		new_filter->children = std::move(op->children);
		op->children.clear();
		op->children.push_back(std::move(new_filter));
	}

	return op;
}

} // namespace duckdb
