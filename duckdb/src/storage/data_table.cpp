#include "duckdb/storage/data_table.hpp"

#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/common/chrono.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/exception/transaction_exception.hpp"
#include "duckdb/common/helper.hpp"
#include "duckdb/common/types/conflict_manager.hpp"
#include "duckdb/common/types/constraint_conflict_info.hpp"
#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "duckdb/execution/expression_executor.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/parser/constraints/list.hpp"
#include "duckdb/planner/constraints/list.hpp"
#include "duckdb/planner/expression_binder/check_binder.hpp"
#include "duckdb/planner/table_filter.hpp"
#include "duckdb/storage/checkpoint/table_data_writer.hpp"
#include "duckdb/storage/storage_manager.hpp"
#include "duckdb/storage/table/append_state.hpp"
#include "duckdb/storage/table/delete_state.hpp"
#include "duckdb/storage/table/persistent_table_data.hpp"
#include "duckdb/storage/table/row_group.hpp"
#include "duckdb/storage/table/scan_state.hpp"
#include "duckdb/storage/table/standard_column_data.hpp"
#include "duckdb/storage/table/update_state.hpp"
#include "duckdb/storage/table_storage_info.hpp"
#include "duckdb/transaction/duck_transaction.hpp"
#include "duckdb/transaction/transaction_manager.hpp"

namespace duckdb {

DataTableInfo::DataTableInfo(AttachedDatabase &db, shared_ptr<TableIOManager> table_io_manager_p, string schema,
                             string table)
    : table_io_manager(std::move(table_io_manager_p)), schema(std::move(schema)), table(std::move(table)) {
}

void DataTableInfo::InitializeIndexes(ClientContext &context, const char *index_type) {
	indexes.InitializeIndexes(context, *this, index_type);
}

bool DataTableInfo::IsTemporary() const {
	return false;
}

DataTable::DataTable(ClientContext &context, DataTable &parent, ColumnDefinition &new_column, Expression &default_value)
    : info(parent.info), is_root(true) {
	// add the column definitions from this DataTable
	for (auto &column_def : parent.column_definitions) {
		column_definitions.emplace_back(column_def.Copy());
	}
	column_definitions.emplace_back(new_column.Copy());

	// this table replaces the previous table, hence the parent is no longer the root DataTable
	parent.is_root = false;
}

DataTable::DataTable(ClientContext &context, DataTable &parent, idx_t removed_column)
    : info(parent.info), is_root(true) {
	lock_guard<mutex> parent_lock(parent.append_lock);

	for (auto &column_def : parent.column_definitions) {
		column_definitions.emplace_back(column_def.Copy());
	}

	info->InitializeIndexes(context);

	// first check if there are any indexes that exist that point to the removed column
	info->indexes.Scan([&](Index &index) {
		for (auto &column_id : index.GetColumnIds()) {
			if (column_id == removed_column) {
				throw CatalogException("Cannot drop this column: an index depends on it!");
			} else if (column_id > removed_column) {
				throw CatalogException("Cannot drop this column: an index depends on a column after it!");
			}
		}
		return false;
	});

	// erase the column definitions from this DataTable
	D_ASSERT(removed_column < column_definitions.size());
	column_definitions.erase_at(removed_column);

	storage_t storage_idx = 0;
	for (idx_t i = 0; i < column_definitions.size(); i++) {
		auto &col = column_definitions[i];
		col.SetOid(i);
		if (col.Generated()) {
			continue;
		}
		col.SetStorageOid(storage_idx++);
	}

	// alter the row_groups and remove the column from each of them
	this->row_groups = parent.row_groups->RemoveColumn(removed_column);

	// this table replaces the previous table, hence the parent is no longer the root DataTable
	parent.is_root = false;
}

// Alter column to add new constraint
DataTable::DataTable(ClientContext &context, DataTable &parent, unique_ptr<BoundConstraint> constraint)
    : info(parent.info), row_groups(parent.row_groups), is_root(true) {

	lock_guard<mutex> parent_lock(parent.append_lock);
	for (auto &column_def : parent.column_definitions) {
		column_definitions.emplace_back(column_def.Copy());
	}

	info->InitializeIndexes(context);

	// this table replaces the previous table, hence the parent is no longer the root DataTable
	parent.is_root = false;
}

DataTable::DataTable(ClientContext &context, DataTable &parent, idx_t changed_idx, const LogicalType &target_type,
                     const vector<column_t> &bound_columns, Expression &cast_expr)
    : info(parent.info), is_root(true) {
}

vector<LogicalType> DataTable::GetTypes() {
	vector<LogicalType> types;
	for (auto &it : column_definitions) {
		types.push_back(it.Type());
	}
	return types;
}

bool DataTable::IsTemporary() const {
	return info->IsTemporary();
}

AttachedDatabase &DataTable::GetAttached() {
	throw TransactionException("Transaction conflict: adding entries to a table that has been altered!");
}

const vector<ColumnDefinition> &DataTable::Columns() const {
	return column_definitions;
}

TableIOManager &DataTable::GetTableIOManager() {
	return *info->table_io_manager;
}

TableIOManager &TableIOManager::Get(DataTable &table) {
	return table.GetTableIOManager();
}

//===--------------------------------------------------------------------===//
// Scan
//===--------------------------------------------------------------------===//
void DataTable::InitializeScan(TableScanState &state, const vector<column_t> &column_ids,
                               TableFilterSet *table_filters) {
	if (!state.checkpoint_lock) {
		state.checkpoint_lock = make_shared_ptr<CheckpointLock>(info->checkpoint_lock.GetSharedLock());
	}
	state.Initialize(column_ids, table_filters);
	row_groups->InitializeScan(state.table_state, column_ids, table_filters);
}

void DataTable::InitializeScan(DuckTransaction &transaction, TableScanState &state, const vector<column_t> &column_ids,
                               TableFilterSet *table_filters) {
	state.checkpoint_lock = transaction.SharedLockTable(*info);
	auto &local_storage = LocalStorage::Get(transaction);
	InitializeScan(state, column_ids, table_filters);
	local_storage.InitializeScan(*this, state.local_state, table_filters);
}

void DataTable::InitializeScanWithOffset(TableScanState &state, const vector<column_t> &column_ids, idx_t start_row,
                                         idx_t end_row) {
	if (!state.checkpoint_lock) {
		state.checkpoint_lock = make_shared_ptr<CheckpointLock>(info->checkpoint_lock.GetSharedLock());
	}
	state.Initialize(column_ids);
	row_groups->InitializeScanWithOffset(state.table_state, column_ids, start_row, end_row);
}

idx_t DataTable::MaxThreads(ClientContext &context) {
	idx_t parallel_scan_vector_count = Storage::ROW_GROUP_VECTOR_COUNT;
	if (ClientConfig::GetConfig(context).verify_parallelism) {
		parallel_scan_vector_count = 1;
	}
	idx_t parallel_scan_tuple_count = STANDARD_VECTOR_SIZE * parallel_scan_vector_count;
	return GetTotalRows() / parallel_scan_tuple_count + 1;
}

void DataTable::InitializeParallelScan(ClientContext &context, ParallelTableScanState &state) {
}

bool DataTable::NextParallelScan(ClientContext &context, ParallelTableScanState &state, TableScanState &scan_state) {
	if (row_groups->NextParallelScan(context, state.scan_state, scan_state.table_state)) {
		return true;
	}

	return false;
}

void DataTable::Scan(DuckTransaction &transaction, DataChunk &result, TableScanState &state) {
	// scan the persistent segments
	if (state.table_state.Scan(transaction, result)) {
		D_ASSERT(result.size() > 0);
		return;
	}

	// scan the transaction-local segments
	auto &local_storage = LocalStorage::Get(transaction);
	local_storage.Scan(state.local_state, state.GetColumnIds(), result);
}

bool DataTable::CreateIndexScan(TableScanState &state, DataChunk &result, TableScanType type) {
	return state.table_state.ScanCommitted(result, type);
}

//===--------------------------------------------------------------------===//
// Index Methods
//===--------------------------------------------------------------------===//
shared_ptr<DataTableInfo> &DataTable::GetDataTableInfo() {
	return info;
}

void DataTable::InitializeIndexes(ClientContext &context) {
	info->InitializeIndexes(context);
}

bool DataTable::HasIndexes() const {
	return !info->indexes.Empty();
}

void DataTable::AddIndex(unique_ptr<Index> index) {
	info->indexes.AddIndex(std::move(index));
}

bool DataTable::HasForeignKeyIndex(const vector<PhysicalIndex> &keys, ForeignKeyType type) {
	return info->indexes.FindForeignKeyIndex(keys, type) != nullptr;
}

void DataTable::SetIndexStorageInfo(vector<IndexStorageInfo> index_storage_info) {
	info->index_storage_infos = std::move(index_storage_info);
}

void DataTable::VacuumIndexes() {
}

void DataTable::CleanupAppend(transaction_t lowest_transaction, idx_t start, idx_t count) {
	row_groups->CleanupAppend(lowest_transaction, start, count);
}

bool DataTable::IndexNameIsUnique(const string &name) {
	return info->indexes.NameIsUnique(name);
}

string DataTableInfo::GetSchemaName() {
	return schema;
}

string DataTableInfo::GetTableName() {
	lock_guard<mutex> l(name_lock);
	return table;
}

void DataTableInfo::SetTableName(string name) {
	lock_guard<mutex> l(name_lock);
	table = std::move(name);
}

string DataTable::GetTableName() const {
	return info->GetTableName();
}

void DataTable::SetTableName(string new_name) {
	info->SetTableName(std::move(new_name));
}

TableStorageInfo DataTable::GetStorageInfo() {
	TableStorageInfo result;
	result.cardinality = GetTotalRows();
	info->indexes.Scan([&](Index &index) {
		IndexInfo index_info;
		index_info.is_primary = index.IsPrimary();
		index_info.is_unique = index.IsUnique() || index_info.is_primary;
		index_info.is_foreign = index.IsForeign();
		index_info.column_set = index.GetColumnIdSet();
		result.index_info.push_back(std::move(index_info));
		return false;
	});
	return result;
}

//===--------------------------------------------------------------------===//
// Fetch
//===--------------------------------------------------------------------===//
void DataTable::Fetch(DuckTransaction &transaction, DataChunk &result, const vector<column_t> &column_ids,
                      const Vector &row_identifiers, idx_t fetch_count, ColumnFetchState &state) {
	auto lock = info->checkpoint_lock.GetSharedLock();
	row_groups->Fetch(transaction, result, column_ids, row_identifiers, fetch_count, state);
}

//===--------------------------------------------------------------------===//
// Append
//===--------------------------------------------------------------------===//
static void VerifyNotNullConstraint(TableCatalogEntry &table, Vector &vector, idx_t count, const string &col_name) {
	if (!VectorOperations::HasNull(vector, count)) {
		return;
	}

	throw ConstraintException("NOT NULL constraint failed: %s.%s", table.name, col_name);
}

// To avoid throwing an error at SELECT, instead this moves the error detection to INSERT
static void VerifyGeneratedExpressionSuccess(ClientContext &context, TableCatalogEntry &table, DataChunk &chunk,
                                             Expression &expr, column_t index) {
}

static void VerifyCheckConstraint(ClientContext &context, TableCatalogEntry &table, Expression &expr,
                                  DataChunk &chunk) {
}

bool DataTable::IsForeignKeyIndex(const vector<PhysicalIndex> &fk_keys, Index &index, ForeignKeyType fk_type) {
	if (fk_type == ForeignKeyType::FK_TYPE_PRIMARY_KEY_TABLE ? !index.IsUnique() : !index.IsForeign()) {
		return false;
	}
	if (fk_keys.size() != index.GetColumnIds().size()) {
		return false;
	}
	for (auto &fk_key : fk_keys) {
		bool is_found = false;
		for (auto &index_key : index.GetColumnIds()) {
			if (fk_key.index == index_key) {
				is_found = true;
				break;
			}
		}
		if (!is_found) {
			return false;
		}
	}
	return true;
}

// Find the first index that is not null, and did not find a match
static idx_t FirstMissingMatch(const ManagedSelection &matches) {
	idx_t match_idx = 0;

	for (idx_t i = 0; i < matches.Size(); i++) {
		auto match = matches.IndexMapsToLocation(match_idx, i);
		match_idx += match;
		if (!match) {
			// This index is missing in the matches vector
			return i;
		}
	}
	return DConstants::INVALID_INDEX;
}

idx_t LocateErrorIndex(bool is_append, const ManagedSelection &matches) {
	idx_t failed_index = DConstants::INVALID_INDEX;
	if (!is_append) {
		// We expected to find nothing, so the first error is the first match
		failed_index = matches[0];
	} else {
		// We expected to find matches for all of them, so the first missing match is the first error
		return FirstMissingMatch(matches);
	}
	return failed_index;
}

[[noreturn]] static void ThrowForeignKeyConstraintError(idx_t failed_index, bool is_append, Index &conflict_index,
                                                        DataChunk &input) {
	throw ConstraintException("message");
}

bool IsForeignKeyConstraintError(bool is_append, idx_t input_count, const ManagedSelection &matches) {
	if (is_append) {
		// We need to find a match for all values
		return matches.Count() != input_count;
	} else {
		// We should not find any matches
		return matches.Count() != 0;
	}
}

static bool IsAppend(VerifyExistenceType verify_type) {
	return verify_type == VerifyExistenceType::APPEND_FK;
}

void DataTable::VerifyForeignKeyConstraint(const BoundForeignKeyConstraint &bfk, ClientContext &context,
                                           DataChunk &chunk, VerifyExistenceType verify_type) {
	return;
}

void DataTable::VerifyAppendForeignKeyConstraint(const BoundForeignKeyConstraint &bfk, ClientContext &context,
                                                 DataChunk &chunk) {
	VerifyForeignKeyConstraint(bfk, context, chunk, VerifyExistenceType::APPEND_FK);
}

void DataTable::VerifyDeleteForeignKeyConstraint(const BoundForeignKeyConstraint &bfk, ClientContext &context,
                                                 DataChunk &chunk) {
	VerifyForeignKeyConstraint(bfk, context, chunk, VerifyExistenceType::DELETE_FK);
}

void DataTable::VerifyNewConstraint(LocalStorage &local_storage, DataTable &parent, const BoundConstraint &constraint) {
	if (constraint.type != ConstraintType::NOT_NULL) {
		throw NotImplementedException("FIXME: ALTER COLUMN with such constraint is not supported yet");
	}

	parent.row_groups->VerifyNewConstraint(parent, constraint);
	local_storage.VerifyNewConstraint(parent, constraint);
}

bool HasUniqueIndexes(TableIndexList &list) {
	bool has_unique_index = false;
	list.Scan([&](Index &index) {
		if (index.IsUnique()) {
			has_unique_index = true;
			return true;
		}
		return false;
	});
	return has_unique_index;
}

void DataTable::VerifyUniqueIndexes(TableIndexList &indexes, ClientContext &context, DataChunk &chunk,
                                    optional_ptr<ConflictManager> conflict_manager) {
	//! check whether or not the chunk can be inserted into the indexes
	if (!conflict_manager) {
		// Only need to verify that no unique constraints are violated
		return;
	}

	D_ASSERT(conflict_manager);
	// The conflict manager is only provided when a ON CONFLICT clause was provided to the INSERT statement

	idx_t matching_indexes = 0;
	auto &conflict_info = conflict_manager->GetConflictInfo();
	// First we figure out how many indexes match our conflict target
	// So we can optimize accordingly
	indexes.Scan([&](Index &index) {
		matching_indexes += conflict_info.ConflictTargetMatches(index);
		return false;
	});
	conflict_manager->SetMode(ConflictManagerMode::SCAN);
	conflict_manager->SetIndexCount(matching_indexes);
	// First we verify only the indexes that match our conflict target
	unordered_set<Index *> checked_indexes;

	conflict_manager->SetMode(ConflictManagerMode::THROW);
	// Then we scan the other indexes, throwing if they cause conflicts on tuples that were not found during
	// the scan
}

void DataTable::VerifyAppendConstraints(ConstraintState &state, ClientContext &context, DataChunk &chunk,
                                        optional_ptr<ConflictManager> conflict_manager) {
	auto &table = state.table;
}

unique_ptr<ConstraintState>
DataTable::InitializeConstraintState(TableCatalogEntry &table,
                                     const vector<unique_ptr<BoundConstraint>> &bound_constraints) {
	return make_uniq<ConstraintState>(table, bound_constraints);
}

void DataTable::InitializeLocalAppend(LocalAppendState &state, TableCatalogEntry &table, ClientContext &context,
                                      const vector<unique_ptr<BoundConstraint>> &bound_constraints) {
	if (!is_root) {
		throw TransactionException("Transaction conflict: adding entries to a table that has been altered!");
	}
}

void DataTable::LocalAppend(LocalAppendState &state, TableCatalogEntry &table, ClientContext &context, DataChunk &chunk,
                            bool unsafe) {
	if (chunk.size() == 0) {
		return;
	}
	D_ASSERT(chunk.ColumnCount() == table.GetColumns().PhysicalColumnCount());
	if (!is_root) {
		throw TransactionException("Transaction conflict: adding entries to a table that has been altered!");
	}

	chunk.Verify();

	// verify any constraints on the new chunk
	if (!unsafe) {
		VerifyAppendConstraints(*state.constraint_state, context, chunk);
	}

	// append to the transaction local data
	LocalStorage::Append(state, chunk);
}

void DataTable::FinalizeLocalAppend(LocalAppendState &state) {
	LocalStorage::FinalizeAppend(state);
}

OptimisticDataWriter &DataTable::CreateOptimisticWriter(ClientContext &context) {
	throw TransactionException("Transaction conflict: adding entries to a table that has been altered!");
}

void DataTable::FinalizeOptimisticWriter(ClientContext &context, OptimisticDataWriter &writer) {
}

void DataTable::LocalMerge(ClientContext &context, RowGroupCollection &collection) {
}

void DataTable::LocalAppend(TableCatalogEntry &table, ClientContext &context, DataChunk &chunk,
                            const vector<unique_ptr<BoundConstraint>> &bound_constraints) {
	LocalAppendState append_state;
	auto &storage = table.GetStorage();
	storage.InitializeLocalAppend(append_state, table, context, bound_constraints);
	storage.LocalAppend(append_state, table, context, chunk);
	storage.FinalizeLocalAppend(append_state);
}

void DataTable::LocalAppend(TableCatalogEntry &table, ClientContext &context, ColumnDataCollection &collection,
                            const vector<unique_ptr<BoundConstraint>> &bound_constraints) {
	LocalAppendState append_state;
	auto &storage = table.GetStorage();
	storage.InitializeLocalAppend(append_state, table, context, bound_constraints);
	for (auto &chunk : collection.Chunks()) {
		storage.LocalAppend(append_state, table, context, chunk);
	}
	storage.FinalizeLocalAppend(append_state);
}

void DataTable::AppendLock(TableAppendState &state) {
	state.append_lock = unique_lock<mutex>(append_lock);
	if (!is_root) {
		throw TransactionException("Transaction conflict: adding entries to a table that has been altered!");
	}
	state.row_start = NumericCast<row_t>(row_groups->GetTotalRows());
	state.current_row = state.row_start;
}

void DataTable::InitializeAppend(DuckTransaction &transaction, TableAppendState &state) {
	// obtain the append lock for this table
	if (!state.append_lock) {
		throw InternalException("DataTable::AppendLock should be called before DataTable::InitializeAppend");
	}
	row_groups->InitializeAppend(transaction, state);
}

void DataTable::Append(DataChunk &chunk, TableAppendState &state) {
	D_ASSERT(is_root);
	row_groups->Append(chunk, state);
}

void DataTable::FinalizeAppend(DuckTransaction &transaction, TableAppendState &state) {
	row_groups->FinalizeAppend(transaction, state);
}

void DataTable::ScanTableSegment(idx_t row_start, idx_t count, const std::function<void(DataChunk &chunk)> &function) {
	if (count == 0) {
		return;
	}
	idx_t end = row_start + count;

	vector<column_t> column_ids;
	vector<LogicalType> types;
	for (idx_t i = 0; i < this->column_definitions.size(); i++) {
		auto &col = this->column_definitions[i];
		column_ids.push_back(i);
		types.push_back(col.Type());
	}
	DataChunk chunk;

	CreateIndexScanState state;

	InitializeScanWithOffset(state, column_ids, row_start, row_start + count);
	auto row_start_aligned = state.table_state.row_group->start + state.table_state.vector_index * STANDARD_VECTOR_SIZE;

	idx_t current_row = row_start_aligned;
	while (current_row < end) {
		state.table_state.ScanCommitted(chunk, TableScanType::TABLE_SCAN_COMMITTED_ROWS);
		if (chunk.size() == 0) {
			break;
		}
		idx_t end_row = current_row + chunk.size();
		// start of chunk is current_row
		// end of chunk is end_row
		// figure out if we need to write the entire chunk or just part of it
		idx_t chunk_start = MaxValue<idx_t>(current_row, row_start);
		idx_t chunk_end = MinValue<idx_t>(end_row, end);
		D_ASSERT(chunk_start < chunk_end);
		idx_t chunk_count = chunk_end - chunk_start;
		if (chunk_count != chunk.size()) {
			D_ASSERT(chunk_count <= chunk.size());
			// need to slice the chunk before insert
			idx_t start_in_chunk;
			if (current_row >= row_start) {
				start_in_chunk = 0;
			} else {
				start_in_chunk = row_start - current_row;
			}
			SelectionVector sel(start_in_chunk, chunk_count);
			chunk.Slice(sel, chunk_count);
			chunk.Verify();
		}
		function(chunk);
		chunk.Reset();
		current_row = end_row;
	}
}

void DataTable::MergeStorage(RowGroupCollection &data, TableIndexList &,
                             optional_ptr<StorageCommitState> commit_state) {
	row_groups->MergeStorage(data, this, commit_state);
	row_groups->Verify();
}

void DataTable::WriteToLog(WriteAheadLog &log, idx_t row_start, idx_t count,
                           optional_ptr<StorageCommitState> commit_state) {
	log.WriteSetTable(info->schema, info->table);
	if (commit_state) {
		idx_t optimistic_count = 0;
		auto entry = commit_state->GetRowGroupData(*this, row_start, optimistic_count);
		if (entry) {
			D_ASSERT(optimistic_count > 0);
			log.WriteRowGroupData(*entry);
			if (optimistic_count > count) {
				throw InternalException(
				    "Optimistically written count cannot exceed actual count (got %llu, but expected count is %llu)",
				    optimistic_count, count);
			}
			// write any remaining (non-optimistically written) rows to the WAL normally
			row_start += optimistic_count;
			count -= optimistic_count;
			if (count == 0) {
				return;
			}
		}
	}
	ScanTableSegment(row_start, count, [&](DataChunk &chunk) { log.WriteInsert(chunk); });
}

void DataTable::CommitAppend(transaction_t commit_id, idx_t row_start, idx_t count) {
	lock_guard<mutex> lock(append_lock);
	row_groups->CommitAppend(commit_id, row_start, count);
}

void DataTable::RevertAppendInternal(idx_t start_row) {
	D_ASSERT(is_root);
	// revert appends made to row_groups
	row_groups->RevertAppendInternal(start_row);
}

void DataTable::RevertAppend(idx_t start_row, idx_t count) {
	lock_guard<mutex> lock(append_lock);

	// revert any appends to indexes
	if (!info->indexes.Empty()) {
		idx_t current_row_base = start_row;
		row_t row_data[STANDARD_VECTOR_SIZE];
		Vector row_identifiers(LogicalType::ROW_TYPE, data_ptr_cast(row_data));
		idx_t scan_count = MinValue<idx_t>(count, row_groups->GetTotalRows() - start_row);
	}

	// we need to vacuum the indexes to remove any buffers that are now empty
	// due to reverting the appends

	// revert the data table append
	RevertAppendInternal(start_row);
}

//===--------------------------------------------------------------------===//
// Indexes
//===--------------------------------------------------------------------===//
ErrorData DataTable::AppendToIndexes(TableIndexList &indexes, DataChunk &chunk, row_t row_start) {
	ErrorData error;
	if (indexes.Empty()) {
		return error;
	}
	// first generate the vector of row identifiers
	Vector row_identifiers(LogicalType::ROW_TYPE);
	VectorOperations::GenerateSequence(row_identifiers, chunk.size(), row_start, 1);

	return error;
}

ErrorData DataTable::AppendToIndexes(DataChunk &chunk, row_t row_start) {
	D_ASSERT(is_root);
	return AppendToIndexes(info->indexes, chunk, row_start);
}

void DataTable::RemoveFromIndexes(TableAppendState &state, DataChunk &chunk, row_t row_start) {
	D_ASSERT(is_root);
	if (info->indexes.Empty()) {
		return;
	}
	// first generate the vector of row identifiers
	Vector row_identifiers(LogicalType::ROW_TYPE);
	VectorOperations::GenerateSequence(row_identifiers, chunk.size(), row_start, 1);

	// now remove the entries from the indices
	RemoveFromIndexes(state, chunk, row_identifiers);
}

void DataTable::RemoveFromIndexes(TableAppendState &state, DataChunk &chunk, Vector &row_identifiers) {
	D_ASSERT(is_root);
}

void DataTable::RemoveFromIndexes(Vector &row_identifiers, idx_t count) {
	D_ASSERT(is_root);
	row_groups->RemoveFromIndexes(info->indexes, row_identifiers, count);
}

//===--------------------------------------------------------------------===//
// Delete
//===--------------------------------------------------------------------===//
static bool TableHasDeleteConstraints(TableCatalogEntry &table) {
	return false;
}

void DataTable::VerifyDeleteConstraints(TableDeleteState &state, ClientContext &context, DataChunk &chunk) {
	for (auto &constraint : state.constraint_state->bound_constraints) {
		switch (constraint->type) {
		case ConstraintType::NOT_NULL:
		case ConstraintType::CHECK:
		case ConstraintType::UNIQUE:
			break;
		case ConstraintType::FOREIGN_KEY: {
			auto &bfk = *reinterpret_cast<BoundForeignKeyConstraint *>(constraint.get());
			if (bfk.info.type == ForeignKeyType::FK_TYPE_PRIMARY_KEY_TABLE ||
			    bfk.info.type == ForeignKeyType::FK_TYPE_SELF_REFERENCE_TABLE) {
				VerifyDeleteForeignKeyConstraint(bfk, context, chunk);
			}
			break;
		}
		default:
			throw NotImplementedException("Constraint type not implemented!");
		}
	}
}

unique_ptr<TableDeleteState> DataTable::InitializeDelete(TableCatalogEntry &table, ClientContext &context,
                                                         const vector<unique_ptr<BoundConstraint>> &bound_constraints) {
	// initialize indexes (if any)
	info->InitializeIndexes(context);

	vector<LogicalType> types;
	auto result = make_uniq<TableDeleteState>();
	result->has_delete_constraints = TableHasDeleteConstraints(table);
	if (result->has_delete_constraints) {
		// initialize the chunk if there are any constraints to verify
		for (idx_t i = 0; i < column_definitions.size(); i++) {
			result->col_ids.push_back(column_definitions[i].StorageOid());
			types.emplace_back(column_definitions[i].Type());
		}
		result->verify_chunk.Initialize(Allocator::Get(context), types);
		result->constraint_state = make_uniq<ConstraintState>(table, bound_constraints);
	}
	return result;
}

idx_t DataTable::Delete(TableDeleteState &state, ClientContext &context, Vector &row_identifiers, idx_t count) {
	return 0;
}

//===--------------------------------------------------------------------===//
// Update
//===--------------------------------------------------------------------===//
static void CreateMockChunk(vector<LogicalType> &types, const vector<PhysicalIndex> &column_ids, DataChunk &chunk,
                            DataChunk &mock_chunk) {
	// construct a mock DataChunk
	mock_chunk.InitializeEmpty(types);
	for (column_t i = 0; i < column_ids.size(); i++) {
		mock_chunk.data[column_ids[i].index].Reference(chunk.data[i]);
	}
	mock_chunk.SetCardinality(chunk.size());
}

static bool CreateMockChunk(TableCatalogEntry &table, const vector<PhysicalIndex> &column_ids,
                            physical_index_set_t &desired_column_ids, DataChunk &chunk, DataChunk &mock_chunk) {
	idx_t found_columns = 0;
	// check whether the desired columns are present in the UPDATE clause
	for (column_t i = 0; i < column_ids.size(); i++) {
		if (desired_column_ids.find(column_ids[i]) != desired_column_ids.end()) {
			found_columns++;
		}
	}
	if (found_columns == 0) {
		// no columns were found: no need to check the constraint again
		return false;
	}
	if (found_columns != desired_column_ids.size()) {
		// not all columns in UPDATE clause are present!
		// this should not be triggered at all as the binder should add these columns
		throw InternalException("Not all columns required for the CHECK constraint are present in the UPDATED chunk!");
	}
	// construct a mock DataChunk
	return true;
}

void DataTable::VerifyUpdateConstraints(ConstraintState &state, ClientContext &context, DataChunk &chunk,
                                        const vector<PhysicalIndex> &column_ids) {
	// update should not be called for indexed columns!
	// instead update should have been rewritten to delete + update on higher layer
}

unique_ptr<TableUpdateState> DataTable::InitializeUpdate(TableCatalogEntry &table, ClientContext &context,
                                                         const vector<unique_ptr<BoundConstraint>> &bound_constraints) {
	// check that there are no unknown indexes
	info->InitializeIndexes(context);

	auto result = make_uniq<TableUpdateState>();
	result->constraint_state = InitializeConstraintState(table, bound_constraints);
	return result;
}

void DataTable::Update(TableUpdateState &state, ClientContext &context, Vector &row_ids,
                       const vector<PhysicalIndex> &column_ids, DataChunk &updates) {
	D_ASSERT(row_ids.GetType().InternalType() == ROW_TYPE);
	D_ASSERT(column_ids.size() == updates.ColumnCount());
	updates.Verify();

	auto count = updates.size();
	if (count == 0) {
		return;
	}

	if (!is_root) {
		throw TransactionException("Transaction conflict: cannot update a table that has been altered!");
	}

	// first verify that no constraints are violated
	VerifyUpdateConstraints(*state.constraint_state, context, updates, column_ids);

	// now perform the actual update
	Vector max_row_id_vec(Value::BIGINT(MAX_ROW_ID));
	Vector row_ids_slice(LogicalType::BIGINT);
	DataChunk updates_slice;

	SelectionVector sel_local_update(count), sel_global_update(count);
	auto n_local_update = 0;
	auto n_global_update = count - n_local_update;
}

void DataTable::UpdateColumn(TableCatalogEntry &table, ClientContext &context, Vector &row_ids,
                             const vector<column_t> &column_path, DataChunk &updates) {
	D_ASSERT(row_ids.GetType().InternalType() == ROW_TYPE);
	D_ASSERT(updates.ColumnCount() == 1);
	updates.Verify();
	if (updates.size() == 0) {
		return;
	}

	if (!is_root) {
		throw TransactionException("Transaction conflict: cannot update a table that has been altered!");
	}
}

//===--------------------------------------------------------------------===//
// Statistics
//===--------------------------------------------------------------------===//

void DataTable::SetDistinct(column_t column_id, unique_ptr<DistinctStatistics> distinct_stats) {
	D_ASSERT(column_id != COLUMN_IDENTIFIER_ROW_ID);
	row_groups->SetDistinct(column_id, std::move(distinct_stats));
}

//===--------------------------------------------------------------------===//
// Checkpoint
//===--------------------------------------------------------------------===//
unique_ptr<StorageLockKey> DataTable::GetSharedCheckpointLock() {
	return info->checkpoint_lock.GetSharedLock();
}

unique_ptr<StorageLockKey> DataTable::GetCheckpointLock() {
	return info->checkpoint_lock.GetExclusiveLock();
}

void DataTable::Checkpoint(TableDataWriter &writer, Serializer &serializer) {
	// checkpoint each individual row group

	// The row group payload data has been written. Now write:
	//   column stats
	//   row-group pointers
	//   table pointer
	//   index data
}

void DataTable::CommitDropColumn(idx_t index) {
	row_groups->CommitDropColumn(index);
}

idx_t DataTable::ColumnCount() const {
	return column_definitions.size();
}

idx_t DataTable::GetTotalRows() const {
	return row_groups->GetTotalRows();
}

void DataTable::CommitDropTable() {
	// commit a drop of this table: mark all blocks as modified, so they can be reclaimed later on
	row_groups->CommitDropTable();

	// propagate dropping this table to its indexes: frees all index memory
}

//===--------------------------------------------------------------------===//
// GetColumnSegmentInfo
//===--------------------------------------------------------------------===//
vector<ColumnSegmentInfo> DataTable::GetColumnSegmentInfo() {
	auto lock = GetSharedCheckpointLock();
	return row_groups->GetColumnSegmentInfo();
}

} // namespace duckdb
