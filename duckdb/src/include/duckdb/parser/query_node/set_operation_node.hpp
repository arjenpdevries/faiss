//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/query_node/set_operation_node.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/enums/set_operation_type.hpp"
#include "duckdb/parser/parsed_expression.hpp"
#include "duckdb/parser/query_node.hpp"
#include "duckdb/parser/sql_statement.hpp"

namespace duckdb {

class SetOperationNode {
public:
	static constexpr const QueryNodeType TYPE = QueryNodeType::SET_OPERATION_NODE;

public:
	SetOperationNode() {
	}

	//! The type of set operation
	SetOperationType setop_type = SetOperationType::NONE;
	//! whether the ALL modifier was used or not
	bool setop_all = false;
	//! The left side of the set operation
	unique_ptr<QueryNode> left;
	//! The right side of the set operation
	unique_ptr<QueryNode> right;

	const vector<unique_ptr<ParsedExpression>> &GetSelectList() const {
		return left->GetSelectList();
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
	static unique_ptr<QueryNode> Deserialize(Deserializer &source);
};

} // namespace duckdb
