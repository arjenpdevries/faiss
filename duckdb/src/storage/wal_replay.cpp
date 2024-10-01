#include "duckdb/catalog/catalog_entry/duck_index_entry.hpp"
#include "duckdb/catalog/catalog_entry/duck_table_entry.hpp"
#include "duckdb/catalog/catalog_entry/scalar_macro_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/type_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/view_catalog_entry.hpp"
#include "duckdb/common/checksum.hpp"
#include "duckdb/common/printer.hpp"
#include "duckdb/common/serializer/binary_deserializer.hpp"
#include "duckdb/common/serializer/buffered_file_reader.hpp"
#include "duckdb/common/serializer/memory_stream.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/execution/index/art/art.hpp"
#include "duckdb/execution/index/index_type_set.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/config.hpp"
#include "duckdb/main/connection.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/parser/parsed_data/alter_table_info.hpp"
#include "duckdb/parser/parsed_data/create_schema_info.hpp"
#include "duckdb/parser/parsed_data/create_view_info.hpp"
#include "duckdb/parser/parsed_data/drop_info.hpp"
#include "duckdb/planner/binder.hpp"
#include "duckdb/planner/expression_binder/index_binder.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"
#include "duckdb/storage/storage_manager.hpp"
#include "duckdb/storage/table/column_data.hpp"
#include "duckdb/storage/table/delete_state.hpp"
#include "duckdb/storage/write_ahead_log.hpp"
#include "duckdb/transaction/meta_transaction.hpp"

namespace duckdb {

class ReplayState {
public:
	ReplayState(AttachedDatabase &db, ClientContext &context) : db(db), context(context) {
	}

	AttachedDatabase &db;
	ClientContext &context;
	optional_ptr<TableCatalogEntry> current_table;
	MetaBlockPointer checkpoint_id;
	idx_t wal_version = 1;
};

class WriteAheadLogDeserializer {
public:
	WriteAheadLogDeserializer(ReplayState &state_p, BufferedFileReader &stream_p, bool deserialize_only = false)
	    : state(state_p), db(state.db), context(state.context), data(nullptr), deserialize_only(deserialize_only) {
	}
	WriteAheadLogDeserializer(ReplayState &state_p, unique_ptr<data_t[]> data_p, idx_t size,
	                          bool deserialize_only = false)
	    : state(state_p), db(state.db), context(state.context), data(std::move(data_p)),
	      deserialize_only(deserialize_only) {
	}

	static WriteAheadLogDeserializer Open(ReplayState &state_p, BufferedFileReader &stream,
	                                      bool deserialize_only = false) {
		if (state_p.wal_version == 1) {
			// old WAL versions do not have checksums
			return WriteAheadLogDeserializer(state_p, stream, deserialize_only);
		}
		if (state_p.wal_version != 2) {
			throw IOException("Failed to read WAL of version %llu - can only read version 1 and 2",
			                  state_p.wal_version);
		}
		// read the checksum and size
		auto size = stream.Read<uint64_t>();
		auto stored_checksum = stream.Read<uint64_t>();
		auto offset = stream.CurrentOffset();
		auto file_size = stream.FileSize();

		if (offset + size > file_size) {
			throw SerializationException(
			    "Corrupt WAL file: entry size exceeded remaining data in file at byte position %llu "
			    "(found entry with size %llu bytes, file size %llu bytes)",
			    offset, size, file_size);
		}

		// allocate a buffer and read data into the buffer
		auto buffer = unique_ptr<data_t[]>(new data_t[size]);
		stream.ReadData(buffer.get(), size);

		// compute and verify the checksum
		auto computed_checksum = Checksum(buffer.get(), size);
		if (stored_checksum != computed_checksum) {
			throw IOException("Corrupt WAL file: entry at byte position %llu computed checksum %llu does not match "
			                  "stored checksum %llu",
			                  offset, computed_checksum, stored_checksum);
		}
		return WriteAheadLogDeserializer(state_p, std::move(buffer), size, deserialize_only);
	}

	bool ReplayEntry() {
		return false;
	}

	bool DeserializeOnly() {
		return deserialize_only;
	}

protected:
	void ReplayEntry(WALType wal_type);

	void ReplayVersion();

	void ReplayCreateTable();
	void ReplayDropTable();
	void ReplayAlter();

	void ReplayCreateView();
	void ReplayDropView();

	void ReplayCreateSchema();
	void ReplayDropSchema();

	void ReplayCreateType();
	void ReplayDropType();

	void ReplayCreateSequence();
	void ReplayDropSequence();
	void ReplaySequenceValue();

	void ReplayCreateMacro();
	void ReplayDropMacro();

	void ReplayCreateTableMacro();
	void ReplayDropTableMacro();

	void ReplayCreateIndex();
	void ReplayDropIndex();

	void ReplayUseTable();
	void ReplayInsert();
	void ReplayRowGroupData();
	void ReplayDelete();
	void ReplayUpdate();
	void ReplayCheckpoint();

private:
	ReplayState &state;
	AttachedDatabase &db;
	ClientContext &context;
	unique_ptr<data_t[]> data;
	bool deserialize_only;
};

//===--------------------------------------------------------------------===//
// Replay
//===--------------------------------------------------------------------===//
bool WriteAheadLog::Replay(AttachedDatabase &database, unique_ptr<FileHandle> handle) {
	Connection con(database.GetDatabase());
	auto wal_path = handle->GetPath();

	con.BeginTransaction();

	auto &config = DBConfig::GetConfig(database.GetDatabase());
	// first deserialize the WAL to look for a checkpoint flag
	// if there is a checkpoint flag, we might have already flushed the contents of the WAL to disk
	ReplayState checkpoint_state(database, *con.context);
	try {
		while (true) {
		}
	} catch (std::exception &ex) { // LCOV_EXCL_START
		ErrorData error(ex);
		// ignore serialization exceptions - they signal a torn WAL
		if (error.Type() != ExceptionType::SERIALIZATION) {
			error.Throw("Failure while replaying WAL file \"" + wal_path + "\": ");
		}
	} // LCOV_EXCL_STOP
	if (checkpoint_state.checkpoint_id.IsValid()) {
		// there is a checkpoint flag: check if we need to deserialize the WAL
		auto &manager = database.GetStorageManager();
		if (manager.IsCheckpointClean(checkpoint_state.checkpoint_id)) {
			// the contents of the WAL have already been checkpointed
			// we can safely truncate the WAL and ignore its contents
			return true;
		}
	}

	// we need to recover from the WAL: actually set up the replay state
	ReplayState state(database, *con.context);

	// reset the reader - we are going to read the WAL from the beginning again

	// replay the WAL
	// note that everything is wrapped inside a try/catch block here
	// there can be errors in WAL replay because of a corrupt WAL file
	try {
		while (true) {
			// read the current entry
		}
	} catch (std::exception &ex) { // LCOV_EXCL_START
		// exception thrown in WAL replay: rollback
		con.Query("ROLLBACK");
		ErrorData error(ex);
		// serialization failure means a truncated WAL
		// these failures are ignored unless abort_on_wal_failure is true
		// other failures always result in an error
		if (config.options.abort_on_wal_failure || error.Type() != ExceptionType::SERIALIZATION) {
			error.Throw("Failure while replaying WAL file \"" + wal_path + "\": ");
		}
	} catch (...) {
		// exception thrown in WAL replay: rollback
		con.Query("ROLLBACK");
		throw;
	} // LCOV_EXCL_STOP
	return false;
}

//===--------------------------------------------------------------------===//
// Replay Entries
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayEntry(WALType entry_type) {
	switch (entry_type) {
	case WALType::WAL_VERSION:
		ReplayVersion();
		break;
	case WALType::CREATE_TABLE:
		ReplayCreateTable();
		break;
	case WALType::DROP_TABLE:
		ReplayDropTable();
		break;
	case WALType::ALTER_INFO:
		ReplayAlter();
		break;
	case WALType::CREATE_VIEW:
		ReplayCreateView();
		break;
	case WALType::DROP_VIEW:
		ReplayDropView();
		break;
	case WALType::CREATE_SCHEMA:
		ReplayCreateSchema();
		break;
	case WALType::DROP_SCHEMA:
		ReplayDropSchema();
		break;
	case WALType::CREATE_SEQUENCE:
		ReplayCreateSequence();
		break;
	case WALType::DROP_SEQUENCE:
		ReplayDropSequence();
		break;
	case WALType::SEQUENCE_VALUE:
		ReplaySequenceValue();
		break;
	case WALType::CREATE_MACRO:
		ReplayCreateMacro();
		break;
	case WALType::DROP_MACRO:
		ReplayDropMacro();
		break;
	case WALType::CREATE_TABLE_MACRO:
		ReplayCreateTableMacro();
		break;
	case WALType::DROP_TABLE_MACRO:
		ReplayDropTableMacro();
		break;
	case WALType::CREATE_INDEX:
		ReplayCreateIndex();
		break;
	case WALType::DROP_INDEX:
		ReplayDropIndex();
		break;
	case WALType::USE_TABLE:
		ReplayUseTable();
		break;
	case WALType::INSERT_TUPLE:
		ReplayInsert();
		break;
	case WALType::ROW_GROUP_DATA:
		ReplayRowGroupData();
		break;
	case WALType::DELETE_TUPLE:
		ReplayDelete();
		break;
	case WALType::UPDATE_TUPLE:
		ReplayUpdate();
		break;
	case WALType::CHECKPOINT:
		ReplayCheckpoint();
		break;
	case WALType::CREATE_TYPE:
		ReplayCreateType();
		break;
	case WALType::DROP_TYPE:
		ReplayDropType();
		break;
	default:
		throw InternalException("Invalid WAL entry type!");
	}
}

//===--------------------------------------------------------------------===//
// Replay Version
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayVersion() {
}

//===--------------------------------------------------------------------===//
// Replay Table
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayCreateTable() {
}

void WriteAheadLogDeserializer::ReplayDropTable() {
	DropInfo info;

	info.type = CatalogType::TABLE_ENTRY;
	if (DeserializeOnly()) {
		return;
	}
}

void WriteAheadLogDeserializer::ReplayAlter() {
}

//===--------------------------------------------------------------------===//
// Replay View
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayCreateView() {
}

void WriteAheadLogDeserializer::ReplayDropView() {
}

//===--------------------------------------------------------------------===//
// Replay Schema
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayCreateSchema() {
}

void WriteAheadLogDeserializer::ReplayDropSchema() {
}

//===--------------------------------------------------------------------===//
// Replay Custom Type
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayCreateType() {
}

void WriteAheadLogDeserializer::ReplayDropType() {
	DropInfo info;

	info.type = CatalogType::TYPE_ENTRY;
	if (DeserializeOnly()) {
		return;
	}
}

//===--------------------------------------------------------------------===//
// Replay Sequence
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayCreateSequence() {
}

void WriteAheadLogDeserializer::ReplayDropSequence() {
}

void WriteAheadLogDeserializer::ReplaySequenceValue() {
}

//===--------------------------------------------------------------------===//
// Replay Macro
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayCreateMacro() {
}

void WriteAheadLogDeserializer::ReplayDropMacro() {
}

//===--------------------------------------------------------------------===//
// Replay Table Macro
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayCreateTableMacro() {
}

void WriteAheadLogDeserializer::ReplayDropTableMacro() {
}

//===--------------------------------------------------------------------===//
// Replay Index
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayCreateIndex() {
}

void WriteAheadLogDeserializer::ReplayDropIndex() {
}

//===--------------------------------------------------------------------===//
// Replay Data
//===--------------------------------------------------------------------===//
void WriteAheadLogDeserializer::ReplayUseTable() {
}

void WriteAheadLogDeserializer::ReplayInsert() {
}

static void MarkBlocksAsUsed(BlockManager &manager, const PersistentColumnData &col_data) {
	for (auto &child_column : col_data.child_columns) {
		MarkBlocksAsUsed(manager, child_column);
	}
}

void WriteAheadLogDeserializer::ReplayRowGroupData() {
	auto &block_manager = db.GetStorageManager().GetBlockManager();
	if (DeserializeOnly()) {
		// label blocks in data as used - they will be used after the WAL replay is finished
		// we need to do this during the deserialization phase to ensure the blocks will not be overwritten
		// by previous deserialization steps
		return;
	}
	if (!state.current_table) {
		throw InternalException("Corrupt WAL: insert without table");
	}
	auto &storage = state.current_table->GetStorage();
}

void WriteAheadLogDeserializer::ReplayDelete() {
	DataChunk chunk;
	if (DeserializeOnly()) {
		return;
	}
	if (!state.current_table) {
		throw InternalException("Corrupt WAL: delete without table");
	}

	D_ASSERT(chunk.ColumnCount() == 1 && chunk.data[0].GetType() == LogicalType::ROW_TYPE);
	row_t row_ids[1];
	Vector row_identifiers(LogicalType::ROW_TYPE, data_ptr_cast(row_ids));

	auto source_ids = FlatVector::GetData<row_t>(chunk.data[0]);
	// delete the tuples from the current table
	TableDeleteState delete_state;
	for (idx_t i = 0; i < chunk.size(); i++) {
		row_ids[0] = source_ids[i];
	}
}

void WriteAheadLogDeserializer::ReplayUpdate() {
}

void WriteAheadLogDeserializer::ReplayCheckpoint() {
}

} // namespace duckdb
