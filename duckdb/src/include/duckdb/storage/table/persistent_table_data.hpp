//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/table/persistent_table_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/constants.hpp"
#include "duckdb/common/vector.hpp"
#include "duckdb/storage/data_pointer.hpp"
#include "duckdb/storage/metadata/metadata_manager.hpp"
#include "duckdb/storage/table/table_statistics.hpp"

namespace duckdb {
class BaseStatistics;

class PersistentTableData {
public:
	explicit PersistentTableData(idx_t column_count);
	~PersistentTableData();

	idx_t total_rows;
	idx_t row_group_count;
	MetaBlockPointer block_pointer;
};

} // namespace duckdb
