
#include "duckdb/execution/expression_executor.hpp"
#include "duckdb/optimizer/rule/in_clause_simplification.hpp"
#include "duckdb/planner/expression/bound_operator_expression.hpp"
#include "duckdb/planner/expression/list.hpp"

namespace duckdb {

InClauseSimplificationRule::InClauseSimplificationRule(ExpressionRewriter &rewriter) : Rule(rewriter) {
	// match on InClauseExpression that has a ConstantExpression as a check
	auto op = make_uniq<InClauseExpressionMatcher>();
	op->policy = SetMatcher::Policy::SOME;
	root = std::move(op);
}

unique_ptr<Expression> InClauseSimplificationRule::Apply(LogicalOperator &op, vector<reference<Expression>> &bindings,
                                                         bool &changes_made, bool is_root) {
	auto &expr = bindings[0].get().Cast<BoundOperatorExpression>();
	if (expr.children[0]->expression_class != ExpressionClass::BOUND_CAST) {
		return nullptr;
	}
	return nullptr;
}

} // namespace duckdb
