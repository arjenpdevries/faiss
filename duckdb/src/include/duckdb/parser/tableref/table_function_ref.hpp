//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/tableref/table_function_ref.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/vector.hpp"
#include "duckdb/parser/parsed_expression.hpp"
#include "duckdb/parser/statement/select_statement.hpp"
#include "duckdb/parser/tableref.hpp"

namespace duckdb {
//! Represents a Table producing function
class TableFunctionRef {
public:
	static constexpr const TableReferenceType TYPE = TableReferenceType::TABLE_FUNCTION;

public:
	unique_ptr<ParsedExpression> function;

	// if the function takes a subquery as argument its in here
	unique_ptr<SelectStatement> subquery;

public:
	string ToString() const;

	bool Equals(const TableRef &other_p) const;

	unique_ptr<TableRef> Copy();

	//! Deserializes a blob back into a BaseTableRef
	void Serialize(Serializer &serializer) const;
	static unique_ptr<TableRef> Deserialize(Deserializer &source);
};
} // namespace duckdb
