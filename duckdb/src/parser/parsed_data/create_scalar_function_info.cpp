#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"

#include "duckdb/parser/parsed_data/alter_scalar_function_info.hpp"

namespace duckdb {

CreateScalarFunctionInfo::CreateScalarFunctionInfo(ScalarFunction function)
    : CreateFunctionInfo(CatalogType::SCALAR_FUNCTION_ENTRY) {
	name = function.name;
	internal = true;
}
CreateScalarFunctionInfo::CreateScalarFunctionInfo(ScalarFunctionSet set)
    : CreateFunctionInfo(CatalogType::SCALAR_FUNCTION_ENTRY) {
}

unique_ptr<CreateInfo> CreateScalarFunctionInfo::Copy() const {
	return nullptr;
}

unique_ptr<AlterInfo> CreateScalarFunctionInfo::GetAlterInfo() const {
	return nullptr;
}

} // namespace duckdb
