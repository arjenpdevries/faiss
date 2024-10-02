//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/parsed_data/create_table_function_info.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/parser/parsed_data/create_function_info.hpp"

namespace duckdb {

struct CreateTableFunctionInfo : public CreateFunctionInfo {
	DUCKDB_API explicit CreateTableFunctionInfo(TableFunction function);
	DUCKDB_API explicit CreateTableFunctionInfo(TableFunctionSet set);

public:
	DUCKDB_API unique_ptr<CreateInfo> Copy() const override;
	DUCKDB_API unique_ptr<AlterInfo> GetAlterInfo() const override;
};

} // namespace duckdb
