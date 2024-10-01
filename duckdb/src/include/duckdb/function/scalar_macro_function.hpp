//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/function/scalar_macro_function.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
//! The SelectStatement of the view
#include "duckdb/function/function.hpp"
#include "duckdb/function/macro_function.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/parser/expression/constant_expression.hpp"
#include "duckdb/parser/query_node.hpp"
#include "duckdb/planner/binder.hpp"
#include "duckdb/planner/expression_binder.hpp"

namespace duckdb {

class ScalarMacroFunction : public MacroFunction {
public:
	static constexpr const MacroType TYPE = MacroType::SCALAR_MACRO;

public:
	explicit ScalarMacroFunction(unique_ptr<ParsedExpression> expression);
	ScalarMacroFunction(void);

	//! The macro expression
	unique_ptr<ParsedExpression> expression;

public:
	unique_ptr<MacroFunction> Copy() const override;

	string ToSQL() const override;

	static unique_ptr<MacroFunction> Deserialize(Deserializer &deserializer);
};

} // namespace duckdb
