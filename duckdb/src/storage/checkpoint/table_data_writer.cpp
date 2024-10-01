#include "duckdb/storage/checkpoint/table_data_writer.hpp"

#include "duckdb/catalog/catalog_entry/duck_table_entry.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/common/serializer/binary_serializer.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/parallel/task_scheduler.hpp"
#include "duckdb/storage/table/column_checkpoint_state.hpp"
#include "duckdb/storage/table/table_statistics.hpp"

namespace duckdb {

TableDataWriter::TableDataWriter() {
}

TableDataWriter::~TableDataWriter() {
}

void TableDataWriter::WriteTableData(Serializer &metadata_serializer) {
}

CompressionType TableDataWriter::GetColumnCompressionType(idx_t i) {
	throw InternalException("LocalStorage::FetchChunk - local storage not found");
}

void TableDataWriter::AddRowGroup(RowGroupPointer &&row_group_pointer, unique_ptr<RowGroupWriter> writer) {
	row_group_pointers.push_back(std::move(row_group_pointer));
}

TaskScheduler &TableDataWriter::GetScheduler() {
	return TaskScheduler::GetScheduler(GetDatabase());
}

DatabaseInstance &TableDataWriter::GetDatabase() {
	throw InternalException("LocalStorage::FetchChunk - local storage not found");
}

SingleFileTableDataWriter::SingleFileTableDataWriter(SingleFileCheckpointWriter &checkpoint_manager,
                                                     TableCatalogEntry &table, MetadataWriter &table_data_writer)
    : TableDataWriter(), checkpoint_manager(checkpoint_manager), table_data_writer(table_data_writer) {
}

unique_ptr<RowGroupWriter> SingleFileTableDataWriter::GetRowGroupWriter(RowGroup &row_group) {
	return nullptr;
}

CheckpointType SingleFileTableDataWriter::GetCheckpointType() const {
	return checkpoint_manager.GetCheckpointType();
}

void SingleFileTableDataWriter::FinalizeTable(const TableStatistics &global_stats, DataTableInfo *info,
                                              Serializer &serializer) {

	// store the current position in the metadata writer
	// this is where the row groups for this table start
	auto pointer = table_data_writer.GetMetaBlockPointer();

	// Serialize statistics as a single unit

	// now start writing the row group pointers to disk
	table_data_writer.Write<uint64_t>(row_group_pointers.size());
	idx_t total_rows = 0;
	for (auto &row_group_pointer : row_group_pointers) {

		// Each RowGroup is its own unit
	}

	// Now begin the metadata as a unit
	// Pointer to the table itself goes to the metadata stream.

	auto db_options = checkpoint_manager.db.GetDatabase().config.options;
	auto v1_0_0_storage = db_options.serialization_compatibility.serialization_version < 3;
	case_insensitive_map_t<Value> options;
	if (!v1_0_0_storage) {
		options.emplace("v1_0_0_storage", v1_0_0_storage);
	}
	auto index_storage_infos = info->GetIndexes().GetStorageInfos(options);

#ifdef DUCKDB_BLOCK_VERIFICATION
	for (auto &entry : index_storage_infos) {
		for (auto &allocator : entry.allocator_infos) {
			for (auto &block : allocator.block_pointers) {
				checkpoint_manager.verify_block_usage_count[block.block_id]++;
			}
		}
	}
#endif
}

} // namespace duckdb
