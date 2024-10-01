#include "duckdb/storage/storage_manager.hpp"

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/common/file_system.hpp"
#include "duckdb/common/serializer/buffered_file_reader.hpp"
#include "duckdb/common/serializer/memory_stream.hpp"
#include "duckdb/function/function.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/main/database_manager.hpp"
#include "duckdb/storage/checkpoint_manager.hpp"
#include "duckdb/storage/in_memory_block_manager.hpp"
#include "duckdb/storage/object_cache.hpp"
#include "duckdb/storage/single_file_block_manager.hpp"
#include "duckdb/storage/storage_extension.hpp"
#include "duckdb/storage/table/column_data.hpp"
#include "duckdb/transaction/transaction_manager.hpp"

namespace duckdb {

StorageManager::StorageManager(AttachedDatabase &db, string path_p, bool read_only)
    : path(std::move(path_p)), read_only(read_only) {
}

StorageManager::~StorageManager() {
}

StorageManager &StorageManager::Get(AttachedDatabase &db) {
	return db.GetStorageManager();
}
StorageManager &StorageManager::Get(Catalog &catalog) {
	throw CatalogException("Cannot launch in-memory database in read-only mode!");
}

DatabaseInstance &StorageManager::GetDatabase() {
	throw CatalogException("Cannot launch in-memory database in read-only mode!");
}

BufferManager &BufferManager::GetBufferManager(ClientContext &context) {
	return BufferManager::GetBufferManager(*context.db);
}

const BufferManager &BufferManager::GetBufferManager(const ClientContext &context) {
	return BufferManager::GetBufferManager(*context.db);
}

ObjectCache &ObjectCache::GetObjectCache(ClientContext &context) {
	return context.db->GetObjectCache();
}

bool ObjectCache::ObjectCacheEnabled(ClientContext &context) {
	return context.db->config.options.object_cache_enable;
}

idx_t StorageManager::GetWALSize() {
	auto wal_ptr = GetWAL();
	if (!wal_ptr) {
		return 0;
	}
	return wal_ptr->GetWALSize();
}

optional_ptr<WriteAheadLog> StorageManager::GetWAL() {
	if (InMemory() || read_only || !load_complete) {
		return nullptr;
	}

	return wal.get();
}

void StorageManager::ResetWAL() {
	auto wal_ptr = GetWAL();
	if (wal_ptr) {
		wal_ptr->Delete();
	}
	wal.reset();
}

string StorageManager::GetWALPath() {
	// we append the ".wal" **before** a question mark in case of GET parameters
	// but only if we are not in a windows long path (which starts with \\?\)
	std::size_t question_mark_pos = std::string::npos;
	if (!StringUtil::StartsWith(path, "\\\\?\\")) {
		question_mark_pos = path.find('?');
	}
	auto wal_path = path;
	if (question_mark_pos != std::string::npos) {
		wal_path.insert(question_mark_pos, ".wal");
	} else {
		wal_path += ".wal";
	}
	return wal_path;
}

bool StorageManager::InMemory() {
	D_ASSERT(!path.empty());
	return path == IN_MEMORY_PATH;
}

void StorageManager::Initialize(const optional_idx block_alloc_size) {
	bool in_memory = InMemory();
	if (in_memory && read_only) {
		throw CatalogException("Cannot launch in-memory database in read-only mode!");
	}

	// Create or load the database from disk, if not in-memory mode.
	LoadDatabase(block_alloc_size);
}

///////////////////////////////////////////////////////////////////////////
class SingleFileTableIOManager : public TableIOManager {
public:
	explicit SingleFileTableIOManager(BlockManager &block_manager) : block_manager(block_manager) {
	}

	BlockManager &block_manager;

public:
	BlockManager &GetIndexBlockManager() override {
		return block_manager;
	}
	BlockManager &GetBlockManagerForRowData() override {
		return block_manager;
	}
	MetadataManager &GetMetadataManager() override {
		return block_manager.GetMetadataManager();
	}
};

SingleFileStorageManager::SingleFileStorageManager(AttachedDatabase &db, string path, bool read_only)
    : StorageManager(db, std::move(path), read_only) {
}

void SingleFileStorageManager::LoadDatabase(const optional_idx block_alloc_size) {

	StorageManagerOptions options;
	options.read_only = read_only;

	// Check if the database file already exists.
	// Note: a file can also exist if there was a ROLLBACK on a previous transaction creating that file.

	load_complete = true;
}

///////////////////////////////////////////////////////////////////////////////

enum class WALCommitState { IN_PROGRESS, FLUSHED, TRUNCATED };

struct OptimisticallyWrittenRowGroupData {
	OptimisticallyWrittenRowGroupData(idx_t start, idx_t count, unique_ptr<PersistentCollectionData> row_group_data_p)
	    : start(start), count(count), row_group_data(std::move(row_group_data_p)) {
	}

	idx_t start;
	idx_t count;
	unique_ptr<PersistentCollectionData> row_group_data;
};

class SingleFileStorageCommitState : public StorageCommitState {
public:
	SingleFileStorageCommitState(StorageManager &storage, WriteAheadLog &log);
	~SingleFileStorageCommitState() override;

	//! Revert the commit
	void RevertCommit() override;
	// Make the commit persistent
	void FlushCommit() override;

	void AddRowGroupData(DataTable &table, idx_t start_index, idx_t count,
	                     unique_ptr<PersistentCollectionData> row_group_data) override;
	optional_ptr<PersistentCollectionData> GetRowGroupData(DataTable &table, idx_t start_index, idx_t &count) override;
	bool HasRowGroupData() override;

private:
	idx_t initial_wal_size = 0;
	idx_t initial_written = 0;
	WriteAheadLog &wal;
	WALCommitState state;
};

SingleFileStorageCommitState::SingleFileStorageCommitState(StorageManager &storage, WriteAheadLog &wal)
    : wal(wal), state(WALCommitState::IN_PROGRESS) {
	auto initial_size = storage.GetWALSize();
	initial_written = wal.GetTotalWritten();
	initial_wal_size = initial_size;
}

SingleFileStorageCommitState::~SingleFileStorageCommitState() {
	if (state != WALCommitState::IN_PROGRESS) {
		return;
	}
	try {
		// truncate the WAL in case of a destructor
		RevertCommit();
	} catch (...) { // NOLINT
	}
}

void SingleFileStorageCommitState::RevertCommit() {
	if (state != WALCommitState::IN_PROGRESS) {
		return;
	}
	if (wal.GetTotalWritten() > initial_written) {
		// remove any entries written into the WAL by truncating it
		wal.Truncate(initial_wal_size);
	}
	state = WALCommitState::TRUNCATED;
}

void SingleFileStorageCommitState::FlushCommit() {
	if (state != WALCommitState::IN_PROGRESS) {
		return;
	}
	wal.Flush();
	state = WALCommitState::FLUSHED;
}

void SingleFileStorageCommitState::AddRowGroupData(DataTable &table, idx_t start_index, idx_t count,
                                                   unique_ptr<PersistentCollectionData> row_group_data) {
}

optional_ptr<PersistentCollectionData> SingleFileStorageCommitState::GetRowGroupData(DataTable &table,
                                                                                     idx_t start_index, idx_t &count) {
	return nullptr;
}

bool SingleFileStorageCommitState::HasRowGroupData() {
	return false;
}

unique_ptr<StorageCommitState> SingleFileStorageManager::GenStorageCommitState(WriteAheadLog &wal) {
	return make_uniq<SingleFileStorageCommitState>(*this, wal);
}

bool SingleFileStorageManager::IsCheckpointClean(MetaBlockPointer checkpoint_id) {
	return block_manager->IsRootBlock(checkpoint_id);
}

void SingleFileStorageManager::CreateCheckpoint(CheckpointOptions options) {
	if (InMemory() || read_only || !load_complete) {
		return;
	}
	if (options.wal_action == CheckpointWALAction::DELETE_WAL) {
		ResetWAL();
	}
}

DatabaseSize SingleFileStorageManager::GetDatabaseSize() {
	// All members default to zero
	DatabaseSize ds;
	if (!InMemory()) {
		ds.total_blocks = block_manager->TotalBlocks();
		ds.block_size = block_manager->GetBlockAllocSize();
		ds.free_blocks = block_manager->FreeBlocks();
		ds.used_blocks = ds.total_blocks - ds.free_blocks;
		ds.bytes = (ds.total_blocks * ds.block_size);
		ds.wal_size = NumericCast<idx_t>(GetWALSize());
	}
	return ds;
}

vector<MetadataBlockInfo> SingleFileStorageManager::GetMetadataInfo() {
	auto &metadata_manager = block_manager->GetMetadataManager();
	return metadata_manager.GetMetadataInfo();
}

bool SingleFileStorageManager::AutomaticCheckpoint(idx_t estimated_wal_bytes) {
	return false;
}

shared_ptr<TableIOManager> SingleFileStorageManager::GetTableIOManager(BoundCreateTableInfo *info /*info*/) {
	// This is an unmanaged reference. No ref/deref overhead. Lifetime of the
	// TableIoManager follows lifetime of the StorageManager (this).
	return shared_ptr<TableIOManager>(shared_ptr<char>(nullptr), table_io_manager.get());
}

BlockManager &SingleFileStorageManager::GetBlockManager() {
	return *block_manager;
}

} // namespace duckdb
