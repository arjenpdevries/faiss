//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/tableref/emptytableref.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/parser/tableref.hpp"

namespace duckdb {

class EmptyTableRef {
public:
	static constexpr const TableReferenceType TYPE = TableReferenceType::EMPTY_FROM;

public:
	EmptyTableRef() {
	}

public:
	string ToString() const;
	bool Equals(const TableRef &other_p) const;

	unique_ptr<TableRef> Copy();

	//! Deserializes a blob back into a DummyTableRef
	void Serialize(Serializer &serializer) const;
	static unique_ptr<TableRef> Deserialize(Deserializer &source);
};
} // namespace duckdb
