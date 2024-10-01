//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/table/row_group_segment_tree.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/storage/table/row_group.hpp"
#include "duckdb/storage/table/segment_tree.hpp"

namespace duckdb {
struct DataTableInfo;
class PersistentTableData;
class MetadataReader;

class RowGroupSegmentTree : public SegmentTree<RowGroup, true> {
public:
	explicit RowGroupSegmentTree(RowGroupCollection &collection);
	~RowGroupSegmentTree() override;

	void Initialize(PersistentTableData &data);

protected:
	unique_ptr<RowGroup> LoadSegment() override;

	idx_t current_row_group;
	idx_t max_row_group;
	unique_ptr<MetadataReader> reader;
};

} // namespace duckdb
