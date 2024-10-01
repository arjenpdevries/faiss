#include "duckdb/transaction/commit_state.hpp"

#include "duckdb/catalog/catalog_entry/duck_index_entry.hpp"
#include "duckdb/catalog/catalog_entry/duck_table_entry.hpp"
#include "duckdb/catalog/catalog_entry/scalar_macro_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/type_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/view_catalog_entry.hpp"
#include "duckdb/catalog/duck_catalog.hpp"
#include "duckdb/common/serializer/binary_deserializer.hpp"
#include "duckdb/common/serializer/memory_stream.hpp"
#include "duckdb/storage/table/chunk_info.hpp"
#include "duckdb/storage/table/column_data.hpp"
#include "duckdb/storage/table/row_version_manager.hpp"
#include "duckdb/storage/table/update_segment.hpp"
#include "duckdb/storage/write_ahead_log.hpp"
#include "duckdb/transaction/append_info.hpp"
#include "duckdb/transaction/delete_info.hpp"
#include "duckdb/transaction/update_info.hpp"

namespace duckdb {

CommitState::CommitState(transaction_t commit_id) : commit_id(commit_id) {
}

void CommitState::CommitEntryDrop(CatalogEntry &entry, data_ptr_t dataptr) {
	return;
}

void CommitState::CommitEntry(UndoFlags type, data_ptr_t data) {
	switch (type) {
	case UndoFlags::CATALOG_ENTRY: {
		// set the commit timestamp of the catalog entry to the given id
		auto catalog_entry = Load<CatalogEntry *>(data);
		D_ASSERT(catalog_entry->HasParent());

		auto &catalog = catalog_entry->ParentCatalog();
		D_ASSERT(catalog.IsDuckCatalog());

		// Grab a write lock on the catalog
		auto &duck_catalog = catalog.Cast<DuckCatalog>();
		lock_guard<mutex> write_lock(duck_catalog.GetWriteLock());

		// drop any blocks associated with the catalog entry if possible (e.g. in case of a DROP or ALTER)
		CommitEntryDrop(*catalog_entry, data + sizeof(CatalogEntry *));
		break;
	}
	case UndoFlags::INSERT_TUPLE: {
		// append:
		auto info = reinterpret_cast<AppendInfo *>(data);
		// mark the tuples as committed
		info->table->CommitAppend(commit_id, info->start_row, info->count);
		break;
	}
	case UndoFlags::DELETE_TUPLE: {
		// deletion:
		auto info = reinterpret_cast<DeleteInfo *>(data);
		// mark the tuples as committed
		info->version_info->CommitDelete(info->vector_idx, commit_id, *info);
		break;
	}
	case UndoFlags::UPDATE_TUPLE: {
		// update:
		auto info = reinterpret_cast<UpdateInfo *>(data);
		info->version_number = commit_id;
		break;
	}
	case UndoFlags::SEQUENCE_VALUE: {
		break;
	}
	default:
		throw InternalException("UndoBuffer - don't know how to commit this type!");
	}
}

void CommitState::RevertCommit(UndoFlags type, data_ptr_t data) {
	transaction_t transaction_id = commit_id;
	switch (type) {
	case UndoFlags::CATALOG_ENTRY: {
		// set the commit timestamp of the catalog entry to the given id
		auto catalog_entry = Load<CatalogEntry *>(data);
		break;
	}
	case UndoFlags::INSERT_TUPLE: {
		auto info = reinterpret_cast<AppendInfo *>(data);
		// revert this append
		info->table->RevertAppend(info->start_row, info->count);
		break;
	}
	case UndoFlags::DELETE_TUPLE: {
		// deletion:
		auto info = reinterpret_cast<DeleteInfo *>(data);
		// revert the commit by writing the (uncommitted) transaction_id back into the version info
		info->version_info->CommitDelete(info->vector_idx, transaction_id, *info);
		break;
	}
	case UndoFlags::UPDATE_TUPLE: {
		// update:
		auto info = reinterpret_cast<UpdateInfo *>(data);
		info->version_number = transaction_id;
		break;
	}
	case UndoFlags::SEQUENCE_VALUE: {
		break;
	}
	default:
		throw InternalException("UndoBuffer - don't know how to revert commit of this type!");
	}
}

} // namespace duckdb
