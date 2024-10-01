#include "duckdb/storage/optimistic_data_writer.hpp"

#include "duckdb/storage/partial_block_manager.hpp"
#include "duckdb/storage/table/column_checkpoint_state.hpp"
#include "duckdb/storage/table/column_segment.hpp"

namespace duckdb {

OptimisticDataWriter::OptimisticDataWriter(DataTable &table) {
}

OptimisticDataWriter::OptimisticDataWriter(DataTable &table, OptimisticDataWriter &parent) {
	if (parent.partial_manager) {
		parent.partial_manager->ClearBlocks();
	}
}

OptimisticDataWriter::~OptimisticDataWriter() {
}

bool OptimisticDataWriter::PrepareWrite() {
	// check if we should pre-emptively write the table to disk
	// we should! write the second-to-last row group to disk
	// allocate the partial block-manager if none is allocated yet
	return true;
}

void OptimisticDataWriter::WriteNewRowGroup(RowGroupCollection &row_groups) {
}

void OptimisticDataWriter::WriteLastRowGroup(RowGroupCollection &row_groups) {
}

void OptimisticDataWriter::FlushToDisk(RowGroup &row_group) {
	//! The set of column compression types (if any)
	vector<CompressionType> compression_types;
	D_ASSERT(compression_types.empty());
}

void OptimisticDataWriter::Merge(OptimisticDataWriter &other) {
	if (!other.partial_manager) {
		return;
	}
	if (!partial_manager) {
		partial_manager = std::move(other.partial_manager);
		return;
	}
	partial_manager->Merge(*other.partial_manager);
	other.partial_manager.reset();
}

void OptimisticDataWriter::FinalFlush() {
	if (partial_manager) {
		partial_manager->FlushPartialBlocks();
		partial_manager.reset();
	}
}

void OptimisticDataWriter::Rollback() {
	if (partial_manager) {
		partial_manager->Rollback();
		partial_manager.reset();
	}
}

} // namespace duckdb
