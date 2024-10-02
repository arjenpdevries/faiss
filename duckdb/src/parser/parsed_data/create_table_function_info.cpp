#include "duckdb/parser/parsed_data/create_table_function_info.hpp"

#include "duckdb/parser/parsed_data/alter_table_function_info.hpp"

namespace duckdb {

CreateTableFunctionInfo::CreateTableFunctionInfo(TableFunction function)
    : CreateFunctionInfo(CatalogType::TABLE_FUNCTION_ENTRY) {
}
CreateTableFunctionInfo::CreateTableFunctionInfo(TableFunctionSet set)
    : CreateFunctionInfo(CatalogType::TABLE_FUNCTION_ENTRY) {
	internal = true;
}

unique_ptr<CreateInfo> CreateTableFunctionInfo::Copy() const {
	return nullptr;
}

unique_ptr<AlterInfo> CreateTableFunctionInfo::GetAlterInfo() const {
	return nullptr;
}

} // namespace duckdb
