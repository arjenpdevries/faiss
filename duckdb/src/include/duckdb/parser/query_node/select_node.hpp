//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/query_node/select_node.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/enums/aggregate_handling.hpp"
#include "duckdb/parser/group_by_node.hpp"
#include "duckdb/parser/parsed_data/sample_options.hpp"
#include "duckdb/parser/parsed_expression.hpp"
#include "duckdb/parser/query_node.hpp"
#include "duckdb/parser/sql_statement.hpp"
#include "duckdb/parser/tableref.hpp"

namespace duckdb {

//! SelectNode represents a standard SELECT statement
class SelectNode {
public:
	static constexpr const QueryNodeType TYPE = QueryNodeType::SELECT_NODE;

public:
	//! The projection list
	vector<unique_ptr<ParsedExpression>> select_list;
	//! The FROM clause
	unique_ptr<TableRef> from_table;
	//! The WHERE clause
	unique_ptr<ParsedExpression> where_clause;
	//! list of groups
	GroupByNode groups;
	//! HAVING clause
	unique_ptr<ParsedExpression> having;
	//! QUALIFY clause
	unique_ptr<ParsedExpression> qualify;
	//! Aggregate handling during binding
	AggregateHandling aggregate_handling;
	//! The SAMPLE clause
	unique_ptr<SampleOptions> sample;

	const vector<unique_ptr<ParsedExpression>> &GetSelectList() const {
		return select_list;
	}

public:
	//! Convert the query node to a string
	string ToString() const;

	bool Equals(const QueryNode *other) const;

	//! Create a copy of this SelectNode
	unique_ptr<QueryNode> Copy() const;

	//! Serializes a QueryNode to a stand-alone binary blob

	//! Deserializes a blob back into a QueryNode

	void Serialize(Serializer &serializer) const;
	static unique_ptr<QueryNode> Deserialize(Deserializer &deserializer);
};

} // namespace duckdb
