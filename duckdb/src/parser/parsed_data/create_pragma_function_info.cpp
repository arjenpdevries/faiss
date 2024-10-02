#include "duckdb/parser/parsed_data/create_pragma_function_info.hpp"

namespace duckdb {

CreatePragmaFunctionInfo::CreatePragmaFunctionInfo(PragmaFunction function)
    : CreateFunctionInfo(CatalogType::PRAGMA_FUNCTION_ENTRY) {
	name = function.name;
	internal = true;
}
CreatePragmaFunctionInfo::CreatePragmaFunctionInfo(string name, PragmaFunctionSet functions_p)
    : CreateFunctionInfo(CatalogType::PRAGMA_FUNCTION_ENTRY) {
	this->name = std::move(name);
	internal = true;
}

unique_ptr<CreateInfo> CreatePragmaFunctionInfo::Copy() const {
	return nullptr;
}

} // namespace duckdb
