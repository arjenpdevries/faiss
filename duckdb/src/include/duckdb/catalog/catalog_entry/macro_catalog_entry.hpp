//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/catalog/catalog_entry/macro_catalog_entry.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/catalog/catalog_entry/function_entry.hpp"
#include "duckdb/catalog/catalog_set.hpp"
#include "duckdb/function/function_set.hpp"
#include "duckdb/function/macro_function.hpp"
#include "duckdb/parser/parsed_data/create_macro_info.hpp"

namespace duckdb {

//! A macro function in the catalog
class MacroCatalogEntry : public FunctionEntry {
public:
	MacroCatalogEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateMacroInfo &info);

	//! The macro function
	vector<unique_ptr<MacroFunction>> macros;

public:
	unique_ptr<CreateInfo> GetInfo() const;

	string ToSQL() const;
};

} // namespace duckdb
