//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/statement/select_statement.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/unordered_map.hpp"
#include "duckdb/parser/parsed_expression.hpp"
#include "duckdb/parser/query_node.hpp"
#include "duckdb/parser/sql_statement.hpp"
#include "duckdb/parser/tableref.hpp"

namespace duckdb {

class QueryNode;
class Serializer;
class Deserializer;

//! SelectStatement is a typical SELECT clause
class SelectStatement {
public:
	static constexpr const StatementType TYPE = StatementType::SELECT_STATEMENT;

public:
	SelectStatement() {
	}

	//! The main query node
	unique_ptr<QueryNode> node;

protected:
	SelectStatement(const SelectStatement &other);

public:
	//! Convert the SELECT statement to a string

	//! Create a copy of this SelectStatement
	DUCKDB_API unique_ptr<SQLStatement> Copy() const;

	void Serialize(Serializer &serializer) const;
	static unique_ptr<SelectStatement> Deserialize(Deserializer &deserializer);
};
} // namespace duckdb
