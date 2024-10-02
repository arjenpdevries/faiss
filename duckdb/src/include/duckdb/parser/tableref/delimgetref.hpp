//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/tableref/delimgetref.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/parser/tableref.hpp"

namespace duckdb {

class DelimGetRef : public TableRef {

public:
	explicit DelimGetRef(const vector<LogicalType> &types_p) : TableRef(), types(types_p) {
	}

	vector<string> internal_aliases;
	vector<LogicalType> types;

public:
	string ToString() const override;
	bool Equals(const TableRef &other_p) const override;

	unique_ptr<TableRef> Copy() override;

	static unique_ptr<TableRef> Deserialize(Deserializer &source);
};
} // namespace duckdb
