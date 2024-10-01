#include "duckdb/transaction/local_storage.hpp"

#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "duckdb/execution/index/art/art.hpp"
#include "duckdb/planner/table_filter.hpp"
#include "duckdb/storage/partial_block_manager.hpp"
#include "duckdb/storage/table/append_state.hpp"
#include "duckdb/storage/table/column_checkpoint_state.hpp"
#include "duckdb/storage/table/row_group.hpp"
#include "duckdb/storage/table/scan_state.hpp"
#include "duckdb/storage/table_io_manager.hpp"
#include "duckdb/storage/write_ahead_log.hpp"
#include "duckdb/transaction/duck_transaction.hpp"

namespace duckdb {

LocalTableStorage::LocalTableStorage(ClientContext &context, DataTable &table)
    : deleted_rows(0), optimistic_writer(table), merged_storage(false) {
}

LocalTableStorage::LocalTableStorage(ClientContext &context, DataTable &new_dt, LocalTableStorage &parent,
                                     idx_t changed_idx, const LogicalType &target_type,
                                     const vector<column_t> &bound_columns, Expression &cast_expr)
    : deleted_rows(parent.deleted_rows), optimistic_writer(new_dt, parent.optimistic_writer),
      optimistic_writers(std::move(parent.optimistic_writers)), merged_storage(parent.merged_storage) {
	indexes.Move(parent.indexes);
}

LocalTableStorage::LocalTableStorage(DataTable &new_dt, LocalTableStorage &parent, idx_t drop_idx)
    : deleted_rows(parent.deleted_rows), optimistic_writer(new_dt, parent.optimistic_writer),
      optimistic_writers(std::move(parent.optimistic_writers)), merged_storage(parent.merged_storage) {
	indexes.Move(parent.indexes);
}

LocalTableStorage::LocalTableStorage(ClientContext &context, DataTable &new_dt, LocalTableStorage &parent,
                                     ColumnDefinition &new_column, ExpressionExecutor &default_executor)
    : deleted_rows(parent.deleted_rows), optimistic_writer(new_dt, parent.optimistic_writer),
      optimistic_writers(std::move(parent.optimistic_writers)), merged_storage(parent.merged_storage) {
	indexes.Move(parent.indexes);
}

LocalTableStorage::~LocalTableStorage() {
}

void LocalTableStorage::InitializeScan(CollectionScanState &state, optional_ptr<TableFilterSet> table_filters) {
}

idx_t LocalTableStorage::EstimatedSize() {
	return 0;
}

void LocalTableStorage::WriteNewRowGroup() {
	if (deleted_rows != 0) {
		// we have deletes - we cannot merge row groups
		return;
	}
}

void LocalTableStorage::FlushBlocks() {
}

ErrorData LocalTableStorage::AppendToIndexes(DuckTransaction &transaction, RowGroupCollection &source,
                                             TableIndexList &index_list, const vector<LogicalType> &table_types,
                                             row_t &start_row) {
	// only need to scan for index append
	// figure out which columns we need to scan for the set of indexes
	auto columns = index_list.GetRequiredColumns();
	// create an empty mock chunk that contains all the correct types for the table
	DataChunk mock_chunk;
	mock_chunk.InitializeEmpty(table_types);
	ErrorData error;
	return error;
}

void LocalTableStorage::AppendToIndexes(DuckTransaction &transaction, TableAppendState &append_state,
                                        idx_t append_count, bool append_to_table) {
}

OptimisticDataWriter &LocalTableStorage::CreateOptimisticWriter() {
	unique_ptr<OptimisticDataWriter> writer = nullptr;
	optimistic_writers.push_back(std::move(writer));
	return *optimistic_writers.back();
}

void LocalTableStorage::FinalizeOptimisticWriter(OptimisticDataWriter &writer) {
	// remove the writer from the set of optimistic writers
	unique_ptr<OptimisticDataWriter> owned_writer;
	for (idx_t i = 0; i < optimistic_writers.size(); i++) {
		if (optimistic_writers[i].get() == &writer) {
			owned_writer = std::move(optimistic_writers[i]);
			optimistic_writers.erase_at(i);
			break;
		}
	}
	if (!owned_writer) {
		throw InternalException("Error in FinalizeOptimisticWriter - could not find writer");
	}
	optimistic_writer.Merge(*owned_writer);
}

void LocalTableStorage::Rollback() {
	for (auto &writer : optimistic_writers) {
		writer->Rollback();
	}
	optimistic_writers.clear();
	optimistic_writer.Rollback();
}

//===--------------------------------------------------------------------===//
// LocalTableManager
//===--------------------------------------------------------------------===//
optional_ptr<LocalTableStorage> LocalTableManager::GetStorage(DataTable &table) {
	lock_guard<mutex> l(table_storage_lock);
	auto entry = table_storage.find(table);
	return entry == table_storage.end() ? nullptr : entry->second.get();
}

LocalTableStorage &LocalTableManager::GetOrCreateStorage(ClientContext &context, DataTable &table) {
	lock_guard<mutex> l(table_storage_lock);
	auto entry = table_storage.find(table);
	if (entry == table_storage.end()) {
		auto new_storage = make_shared_ptr<LocalTableStorage>(context, table);
		auto storage = new_storage.get();
		table_storage.insert(make_pair(reference<DataTable>(table), std::move(new_storage)));
		return *storage;
	} else {
		return *entry->second.get();
	}
}

bool LocalTableManager::IsEmpty() {
	lock_guard<mutex> l(table_storage_lock);
	return table_storage.empty();
}

shared_ptr<LocalTableStorage> LocalTableManager::MoveEntry(DataTable &table) {
	lock_guard<mutex> l(table_storage_lock);
	auto entry = table_storage.find(table);
	if (entry == table_storage.end()) {
		return nullptr;
	}
	auto storage_entry = std::move(entry->second);
	table_storage.erase(entry);
	return storage_entry;
}

reference_map_t<DataTable, shared_ptr<LocalTableStorage>> LocalTableManager::MoveEntries() {
	lock_guard<mutex> l(table_storage_lock);
	return std::move(table_storage);
}

idx_t LocalTableManager::EstimatedSize() {
	lock_guard<mutex> l(table_storage_lock);
	idx_t estimated_size = 0;
	for (auto &storage : table_storage) {
		estimated_size += storage.second->EstimatedSize();
	}
	return estimated_size;
}

void LocalTableManager::InsertEntry(DataTable &table, shared_ptr<LocalTableStorage> entry) {
	lock_guard<mutex> l(table_storage_lock);
	D_ASSERT(table_storage.find(table) == table_storage.end());
	table_storage[table] = std::move(entry);
}

//===--------------------------------------------------------------------===//
// LocalStorage
//===--------------------------------------------------------------------===//
LocalStorage::LocalStorage(ClientContext &context, DuckTransaction &transaction) : context(context) {
}

LocalStorage::CommitState::CommitState() {
}

LocalStorage::CommitState::~CommitState() {
}

LocalStorage &LocalStorage::Get(DuckTransaction &transaction) {
	throw InternalException("LocalStorage::FetchChunk - local storage not found");
}

LocalStorage &LocalStorage::Get(ClientContext &context, AttachedDatabase &db) {
	throw InternalException("LocalStorage::FetchChunk - local storage not found");
}

LocalStorage &LocalStorage::Get(ClientContext &context, Catalog &catalog) {
	throw InternalException("LocalStorage::FetchChunk - local storage not found");
}

void LocalStorage::InitializeScan(DataTable &table, CollectionScanState &state,
                                  optional_ptr<TableFilterSet> table_filters) {
	auto storage = table_manager.GetStorage(table);
	storage->InitializeScan(state, table_filters);
}

void LocalStorage::Scan(CollectionScanState &state, const vector<storage_t> &column_ids, DataChunk &result) {
}

void LocalStorage::InitializeParallelScan(DataTable &table, ParallelCollectionScanState &state) {
	auto storage = table_manager.GetStorage(table);
	if (!storage) {
		state.max_row = 0;
		state.vector_index = 0;
		state.current_row_group = nullptr;
	} else {
	}
}

bool LocalStorage::NextParallelScan(ClientContext &context, DataTable &table, ParallelCollectionScanState &state,
                                    CollectionScanState &scan_state) {
	auto storage = table_manager.GetStorage(table);
	if (!storage) {
		return false;
	}
	return false;
}

void LocalStorage::InitializeAppend(LocalAppendState &state, DataTable &table) {
}

void LocalStorage::Append(LocalAppendState &state, DataChunk &chunk) {
}

void LocalStorage::FinalizeAppend(LocalAppendState &state) {
}

void LocalStorage::LocalMerge(DataTable &table, RowGroupCollection &collection) {
}

OptimisticDataWriter &LocalStorage::CreateOptimisticWriter(DataTable &table) {
	auto &storage = table_manager.GetOrCreateStorage(context, table);
	return storage.CreateOptimisticWriter();
}

void LocalStorage::FinalizeOptimisticWriter(DataTable &table, OptimisticDataWriter &writer) {
	auto &storage = table_manager.GetOrCreateStorage(context, table);
	storage.FinalizeOptimisticWriter(writer);
}

bool LocalStorage::ChangesMade() noexcept {
	return !table_manager.IsEmpty();
}

bool LocalStorage::Find(DataTable &table) {
	return table_manager.GetStorage(table) != nullptr;
}

idx_t LocalStorage::EstimatedSize() {
	return table_manager.EstimatedSize();
}

idx_t LocalStorage::Delete(DataTable &table, Vector &row_ids, idx_t count) {
	auto storage = table_manager.GetStorage(table);
	D_ASSERT(storage);

	// delete from unique indices (if any)

	auto ids = FlatVector::GetData<row_t>(row_ids);
	return 0;
}

void LocalStorage::Update(DataTable &table, Vector &row_ids, const vector<PhysicalIndex> &column_ids,
                          DataChunk &updates) {
}

void LocalStorage::Flush(DataTable &table, LocalTableStorage &storage, optional_ptr<StorageCommitState> commit_state) {
	if (storage.is_dropped) {
		return;
	}
	// possibly vacuum any excess index data
}

void LocalStorage::Commit(optional_ptr<StorageCommitState> commit_state) {
	// commit local storage
	// iterate over all entries in the table storage map and commit them
	// after this, the local storage is no longer required and can be cleared
	auto table_storage = table_manager.MoveEntries();
	for (auto &entry : table_storage) {
		auto table = entry.first;
		auto storage = entry.second.get();
		Flush(table, *storage, commit_state);
		entry.second.reset();
	}
}

void LocalStorage::Rollback() {
	// rollback local storage
	// after this, the local storage is no longer required and can be cleared
	auto table_storage = table_manager.MoveEntries();
	for (auto &entry : table_storage) {
		auto storage = entry.second.get();
		if (!storage) {
			continue;
		}
		storage->Rollback();

		entry.second.reset();
	}
}

idx_t LocalStorage::AddedRows(DataTable &table) {
	auto storage = table_manager.GetStorage(table);
	if (!storage) {
		return 0;
	}
	return 0;
}

void LocalStorage::DropTable(DataTable &table) {
	auto storage = table_manager.GetStorage(table);
	if (!storage) {
		return;
	}
	storage->is_dropped = true;
}

void LocalStorage::MoveStorage(DataTable &old_dt, DataTable &new_dt) {
	// check if there are any pending appends for the old version of the table
	auto new_storage = table_manager.MoveEntry(old_dt);
	if (!new_storage) {
		return;
	}
	// take over the storage from the old entry
	table_manager.InsertEntry(new_dt, std::move(new_storage));
}

void LocalStorage::AddColumn(DataTable &old_dt, DataTable &new_dt, ColumnDefinition &new_column,
                             ExpressionExecutor &default_executor) {
	// check if there are any pending appends for the old version of the table
	auto storage = table_manager.MoveEntry(old_dt);
	if (!storage) {
		return;
	}
	auto new_storage = make_shared_ptr<LocalTableStorage>(context, new_dt, *storage, new_column, default_executor);
	table_manager.InsertEntry(new_dt, std::move(new_storage));
}

void LocalStorage::DropColumn(DataTable &old_dt, DataTable &new_dt, idx_t removed_column) {
	// check if there are any pending appends for the old version of the table
	auto storage = table_manager.MoveEntry(old_dt);
	if (!storage) {
		return;
	}
	auto new_storage = make_shared_ptr<LocalTableStorage>(new_dt, *storage, removed_column);
	table_manager.InsertEntry(new_dt, std::move(new_storage));
}

void LocalStorage::ChangeType(DataTable &old_dt, DataTable &new_dt, idx_t changed_idx, const LogicalType &target_type,
                              const vector<column_t> &bound_columns, Expression &cast_expr) {
	// check if there are any pending appends for the old version of the table
	auto storage = table_manager.MoveEntry(old_dt);
	if (!storage) {
		return;
	}
	auto new_storage = make_shared_ptr<LocalTableStorage>(context, new_dt, *storage, changed_idx, target_type,
	                                                      bound_columns, cast_expr);
	table_manager.InsertEntry(new_dt, std::move(new_storage));
}

void LocalStorage::FetchChunk(DataTable &table, Vector &row_ids, idx_t count, const vector<column_t> &col_ids,
                              DataChunk &chunk, ColumnFetchState &fetch_state) {
	auto storage = table_manager.GetStorage(table);
	if (!storage) {
		throw InternalException("LocalStorage::FetchChunk - local storage not found");
	}
}

TableIndexList &LocalStorage::GetIndexes(DataTable &table) {
	auto storage = table_manager.GetStorage(table);
	if (!storage) {
		throw InternalException("LocalStorage::GetIndexes - local storage not found");
	}
	return storage->indexes;
}

void LocalStorage::VerifyNewConstraint(DataTable &parent, const BoundConstraint &constraint) {
	auto storage = table_manager.GetStorage(parent);
	if (!storage) {
		return;
	}
}

} // namespace duckdb
