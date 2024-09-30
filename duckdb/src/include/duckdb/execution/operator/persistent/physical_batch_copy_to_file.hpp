//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/persistent/physical_fixed_batch_copy.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/file_system.hpp"
#include "duckdb/common/filename_pattern.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/function/copy_function.hpp"
#include "duckdb/parser/parsed_data/copy_info.hpp"

namespace duckdb {
struct FixedRawBatchData;

} // namespace duckdb
