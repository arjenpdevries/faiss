//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/schema/physical_create_art_index.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/index/art/art.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/parser/parsed_data/create_index_info.hpp"
#include "duckdb/storage/data_table.hpp"

#include <fstream>

namespace duckdb {
class DuckTableEntry;

} // namespace duckdb
