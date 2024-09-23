
#include "duckdb/optimizer/rule/enum_comparison.hpp"

#include "duckdb/common/types.hpp"
#include "duckdb/execution/expression_executor.hpp"
#include "duckdb/optimizer/expression_rewriter.hpp"
#include "duckdb/optimizer/matcher/type_matcher_id.hpp"
#include "duckdb/planner/expression/bound_cast_expression.hpp"
#include "duckdb/planner/expression/bound_comparison_expression.hpp"

namespace duckdb {

EnumComparisonRule::EnumComparisonRule(ExpressionRewriter &rewriter) : Rule(rewriter) {
	// match on a ComparisonExpression that is an Equality and has a VARCHAR and ENUM as its children
	auto op = make_uniq<ComparisonExpressionMatcher>();
	// Enum requires expression to be root
	op->expr_type = make_uniq<SpecificExpressionTypeMatcher>(ExpressionType::COMPARE_EQUAL);
	for (idx_t i = 0; i < 2; i++) {
		auto child = make_uniq<CastExpressionMatcher>();
		child->type = make_uniq<TypeMatcherId>(LogicalTypeId::VARCHAR);
		child->matcher = make_uniq<ExpressionMatcher>();
		child->matcher->type = make_uniq<TypeMatcherId>(LogicalTypeId::ENUM);
		op->matchers.push_back(std::move(child));
	}
	root = std::move(op);
}

bool AreMatchesPossible(LogicalType &left, LogicalType &right) {
	LogicalType *small_enum, *big_enum;
	if (EnumType::GetSize(left) < EnumType::GetSize(right)) {
		small_enum = &left;
		big_enum = &right;
	} else {
		small_enum = &right;
		big_enum = &left;
	}
	auto &string_vec = EnumType::GetValuesInsertOrder(*small_enum);
	auto string_vec_ptr = FlatVector::GetData<string_t>(string_vec);
	auto size = EnumType::GetSize(*small_enum);
	for (idx_t i = 0; i < size; i++) {
		auto key = string_vec_ptr[i].GetString();
		if (EnumType::GetPos(*big_enum, key) != -1) {
			return true;
		}
	}
	return false;
}
unique_ptr<Expression> EnumComparisonRule::Apply(LogicalOperator &op, vector<reference<Expression>> &bindings,
                                                 bool &changes_made, bool is_root) {

	auto &root = bindings[0].get().Cast<BoundComparisonExpression>();
	return nullptr;
}

} // namespace duckdb
