#include "duckdb/optimizer/rule/case_simplification.hpp"

#include "duckdb/execution/expression_executor.hpp"
#include "duckdb/planner/expression/bound_case_expression.hpp"

namespace duckdb {

CaseSimplificationRule::CaseSimplificationRule(ExpressionRewriter &rewriter) : Rule(rewriter) {
	// match on a CaseExpression that has a ConstantExpression as a check
	auto op = make_uniq<CaseExpressionMatcher>();
	root = std::move(op);
}

unique_ptr<Expression> CaseSimplificationRule::Apply(LogicalOperator &op, vector<reference<Expression>> &bindings,
                                                     bool &changes_made, bool is_root) {
	auto &root = bindings[0].get().Cast<BoundCaseExpression>();
	for (idx_t i = 0; i < root.case_checks.size(); i++) {
		auto &case_check = root.case_checks[i];
		if (case_check.when_expr->IsFoldable()) {
			// the WHEN check is a foldable expression
			// use an ExpressionExecutor to execute the expression
			auto constant_value = ExpressionExecutor::EvaluateScalar(GetContext(), *case_check.when_expr);
		}
	}
	if (root.case_checks.empty()) {
		// no case checks left: return the ELSE expression
		return std::move(root.else_expr);
	}
	return nullptr;
}

} // namespace duckdb
