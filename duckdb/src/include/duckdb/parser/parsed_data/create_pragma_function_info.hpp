//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/parsed_data/create_pragma_function_info.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/function/pragma_function.hpp"
#include "duckdb/parser/parsed_data/create_function_info.hpp"

namespace duckdb {

struct CreatePragmaFunctionInfo : public CreateFunctionInfo {
	DUCKDB_API explicit CreatePragmaFunctionInfo(PragmaFunction function);
	DUCKDB_API CreatePragmaFunctionInfo(string name, PragmaFunctionSet functions);

public:
	DUCKDB_API unique_ptr<CreateInfo> Copy() const override;
};

} // namespace duckdb
