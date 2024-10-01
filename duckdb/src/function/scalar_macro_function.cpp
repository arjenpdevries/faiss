//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/function/scalar_macro_function.hpp
//
//
//===----------------------------------------------------------------------===//

#include "duckdb/function/scalar_macro_function.hpp"

#include "duckdb/function/macro_function.hpp"
#include "duckdb/parser/expression/constant_expression.hpp"
#include "duckdb/parser/parsed_expression_iterator.hpp"

namespace duckdb {

ScalarMacroFunction::ScalarMacroFunction(unique_ptr<ParsedExpression> expression)
    : MacroFunction(MacroType::SCALAR_MACRO), expression(std::move(expression)) {
}

ScalarMacroFunction::ScalarMacroFunction(void) : MacroFunction(MacroType::SCALAR_MACRO) {
}

unique_ptr<MacroFunction> ScalarMacroFunction::Copy() const {
	auto result = make_uniq<ScalarMacroFunction>();
	result->expression = expression->Copy();
	CopyProperties(*result);

	return std::move(result);
}

void RemoveQualificationRecursive(unique_ptr<ParsedExpression> &expr) {
	if (expr->GetExpressionType() == ExpressionType::COLUMN_REF) {
	} else {
	}
}

string ScalarMacroFunction::ToSQL() const {
	// In case of nested macro's we need to fix it a bit
	auto expression_copy = expression->Copy();
	RemoveQualificationRecursive(expression_copy);
	return MacroFunction::ToSQL() + StringUtil::Format("(%s)", expression_copy->ToString());
}

} // namespace duckdb
