//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/tableref/showref.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/types.hpp"
#include "duckdb/common/vector.hpp"
#include "duckdb/parser/parsed_expression.hpp"
#include "duckdb/parser/query_node.hpp"
#include "duckdb/parser/tableref.hpp"

namespace duckdb {

enum class ShowType : uint8_t { SUMMARY, DESCRIBE };

//! Represents a SHOW/DESCRIBE/SUMMARIZE statement
class ShowRef {
public:
	static constexpr const TableReferenceType TYPE = TableReferenceType::SHOW_REF;

public:
	//! The table name (if any)
	string table_name;
	//! The QueryNode of select query (if any)
	unique_ptr<QueryNode> query;
	//! Whether or not we are requesting a summary or a describe
	ShowType show_type;

public:
	string ToString() const;
	bool Equals(const TableRef &other_p) const;

	unique_ptr<TableRef> Copy();

	//! Deserializes a blob back into a ExpressionListRef
	void Serialize(Serializer &serializer) const;
	static unique_ptr<TableRef> Deserialize(Deserializer &source);
};

} // namespace duckdb
