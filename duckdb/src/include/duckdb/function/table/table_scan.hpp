//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/function/table/table_scan.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/atomic.hpp"
#include "duckdb/function/built_in_functions.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {
class TableCatalogEntry;

struct TableScanBindData : public TableFunctionData {
	explicit TableScanBindData() : is_index_scan(false), is_create_index(false) {
	}

	//! Whether or not the table scan is an index scan.
	bool is_index_scan;
	//! Whether or not the table scan is for index creation.
	bool is_create_index;
	//! The row ids to fetch in case of an index scan.
	unsafe_vector<row_t> row_ids;

public:
	bool Equals(const FunctionData &other_p) const override {
		return false;
	}
};

//! The table scan function represents a sequential scan over one of DuckDB's base tables.
struct TableScanFunction {
	static void RegisterFunction(BuiltinFunctions &set);
	static TableFunction GetFunction();
	static TableFunction GetIndexScanFunction();
};

} // namespace duckdb
