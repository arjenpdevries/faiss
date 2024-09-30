//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/parsed_data/create_type_info.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/parser/column_definition.hpp"
#include "duckdb/parser/constraint.hpp"
#include "duckdb/parser/parsed_data/create_info.hpp"
#include "duckdb/parser/statement/select_statement.hpp"

namespace duckdb {

struct BindTypeModifiersInput {
	ClientContext &context;
	const LogicalType &type;
	const vector<Value> &modifiers;
};

//! The type to bind type modifiers to a type
typedef LogicalType (*bind_type_modifiers_function_t)(BindTypeModifiersInput &input);

struct CreateTypeInfo : public CreateInfo {
	CreateTypeInfo();
	CreateTypeInfo(string name_p, LogicalType type_p, bind_type_modifiers_function_t bind_modifiers_p = nullptr);

	//! Name of the Type
	string name;
	//! Logical Type
	LogicalType type;
	//! Used by create enum from query
	unique_ptr<SQLStatement> query;
	//! Bind type modifiers to the type
	bind_type_modifiers_function_t bind_modifiers;

public:
	unique_ptr<CreateInfo> Copy() const override;

	string ToString() const override;
};

} // namespace duckdb
