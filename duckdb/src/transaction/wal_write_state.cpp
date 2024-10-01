#include "duckdb/transaction/wal_write_state.hpp"

#include "duckdb/catalog/catalog_entry/duck_index_entry.hpp"
#include "duckdb/catalog/catalog_entry/duck_table_entry.hpp"
#include "duckdb/catalog/catalog_entry/scalar_macro_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/type_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/view_catalog_entry.hpp"
#include "duckdb/catalog/catalog_set.hpp"
#include "duckdb/catalog/duck_catalog.hpp"
#include "duckdb/common/serializer/binary_deserializer.hpp"
#include "duckdb/common/serializer/memory_stream.hpp"
#include "duckdb/storage/data_table.hpp"
#include "duckdb/storage/table/chunk_info.hpp"
#include "duckdb/storage/table/column_data.hpp"
#include "duckdb/storage/table/row_group.hpp"
#include "duckdb/storage/table/row_version_manager.hpp"
#include "duckdb/storage/table/update_segment.hpp"
#include "duckdb/storage/write_ahead_log.hpp"
#include "duckdb/transaction/append_info.hpp"
#include "duckdb/transaction/delete_info.hpp"
#include "duckdb/transaction/update_info.hpp"

namespace duckdb {

WALWriteState::WALWriteState(WriteAheadLog &log, optional_ptr<StorageCommitState> commit_state)
    : log(log), commit_state(commit_state), current_table_info(nullptr) {
}

void WALWriteState::SwitchTable(DataTableInfo *table_info, UndoFlags new_op) {
	if (current_table_info != table_info) {
		// write the current table to the log
		log.WriteSetTable(table_info->GetSchemaName(), table_info->GetTableName());
		current_table_info = table_info;
	}
}

void WALWriteState::WriteCatalogEntry(CatalogEntry &entry, data_ptr_t dataptr) {
	return;
}

void WALWriteState::WriteDelete(DeleteInfo &info) {
	// switch to the current table, if necessary
	SwitchTable(info.table->GetDataTableInfo().get(), UndoFlags::DELETE_TUPLE);

	if (!delete_chunk) {
		delete_chunk = make_uniq<DataChunk>();
		vector<LogicalType> delete_types = {LogicalType::ROW_TYPE};
		delete_chunk->Initialize(Allocator::DefaultAllocator(), delete_types);
	}
	auto rows = FlatVector::GetData<row_t>(delete_chunk->data[0]);
	if (info.is_consecutive) {
		for (idx_t i = 0; i < info.count; i++) {
			rows[i] = UnsafeNumericCast<int64_t>(info.base_row + i);
		}
	} else {
		auto delete_rows = info.GetRows();
		for (idx_t i = 0; i < info.count; i++) {
			rows[i] = UnsafeNumericCast<int64_t>(info.base_row) + delete_rows[i];
		}
	}
	delete_chunk->SetCardinality(info.count);
	log.WriteDelete(*delete_chunk);
}

void WALWriteState::WriteUpdate(UpdateInfo &info) {
	// switch to the current table, if necessary
	return;
}

void WALWriteState::CommitEntry(UndoFlags type, data_ptr_t data) {
	switch (type) {
	case UndoFlags::CATALOG_ENTRY: {
		// set the commit timestamp of the catalog entry to the given id
		auto catalog_entry = Load<CatalogEntry *>(data);
		D_ASSERT(catalog_entry->HasParent());
		// push the catalog update to the WAL
		WriteCatalogEntry(*catalog_entry, data + sizeof(CatalogEntry *));
		break;
	}
	case UndoFlags::INSERT_TUPLE: {
		// append:
		auto info = reinterpret_cast<AppendInfo *>(data);
		if (!info->table->IsTemporary()) {
			info->table->WriteToLog(log, info->start_row, info->count, commit_state.get());
		}
		break;
	}
	case UndoFlags::DELETE_TUPLE: {
		// deletion:
		auto info = reinterpret_cast<DeleteInfo *>(data);
		if (!info->table->IsTemporary()) {
			WriteDelete(*info);
		}
		break;
	}
	case UndoFlags::UPDATE_TUPLE: {
		// update:
		auto info = reinterpret_cast<UpdateInfo *>(data);
		break;
	}
	case UndoFlags::SEQUENCE_VALUE: {
		auto info = reinterpret_cast<SequenceValue *>(data);
		log.WriteSequenceValue(*info);
		break;
	}
	default:
		throw InternalException("UndoBuffer - don't know how to commit this type!");
	}
}

} // namespace duckdb
