//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/persistent/physical_export.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/function/copy_function.hpp"
#include "duckdb/parser/parsed_data/copy_info.hpp"
#include "duckdb/parser/parsed_data/exported_table_data.hpp"

#include <utility>

namespace duckdb {

struct ExportEntries {
	vector<reference<CatalogEntry>> schemas;
	vector<reference<CatalogEntry>> custom_types;
	vector<reference<CatalogEntry>> sequences;
	vector<reference<CatalogEntry>> tables;
	vector<reference<CatalogEntry>> views;
	vector<reference<CatalogEntry>> indexes;
	vector<reference<CatalogEntry>> macros;
};

} // namespace duckdb
