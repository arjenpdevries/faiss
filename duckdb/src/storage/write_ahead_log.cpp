#include "duckdb/storage/write_ahead_log.hpp"

#include "duckdb/catalog/catalog_entry/duck_index_entry.hpp"
#include "duckdb/catalog/catalog_entry/scalar_macro_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/type_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/view_catalog_entry.hpp"
#include "duckdb/common/checksum.hpp"
#include "duckdb/common/serializer/binary_serializer.hpp"
#include "duckdb/common/serializer/memory_stream.hpp"
#include "duckdb/execution/index/bound_index.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/parser/parsed_data/alter_table_info.hpp"
#include "duckdb/storage/index.hpp"
#include "duckdb/storage/table/column_data.hpp"
#include "duckdb/storage/table/data_table_info.hpp"
#include "duckdb/storage/table_io_manager.hpp"

namespace duckdb {

const uint64_t WAL_VERSION_NUMBER = 2;

WriteAheadLog::WriteAheadLog(AttachedDatabase &database, const string &wal_path)
    : database(database), wal_path(wal_path), wal_size(0), initialized(false) {
}

WriteAheadLog::~WriteAheadLog() {
}

//! Gets the total bytes written to the WAL since startup
idx_t WriteAheadLog::GetWALSize() {
	if (!Initialized()) {
		auto &fs = FileSystem::Get(database);
		if (!fs.FileExists(wal_path)) {
			return 0;
		}
	}
	return wal_size;
}

idx_t WriteAheadLog::GetTotalWritten() {
	return 0;
}

void WriteAheadLog::Truncate(idx_t size) {
	return;
}

void WriteAheadLog::Delete() {
	if (!Initialized()) {
		return;
	}
	auto &fs = FileSystem::Get(database);
	fs.RemoveFile(wal_path);
	wal_size = 0;
}

//===--------------------------------------------------------------------===//
// Serializer
//===--------------------------------------------------------------------===//
class ChecksumWriter : public WriteStream {
public:
	explicit ChecksumWriter(WriteAheadLog &wal) : wal(wal) {
	}

	void WriteData(const_data_ptr_t buffer, idx_t write_size) override {
	}

	void Flush() {
	}

private:
	WriteAheadLog &wal;
	optional_ptr<WriteStream> stream;
};

class WriteAheadLogSerializer {
public:
	WriteAheadLogSerializer(WriteAheadLog &wal, WALType wal_type) : checksum_writer(wal) {
		// write a version marker if none has been written yet
		wal.WriteVersion();
	}

	void End() {
		checksum_writer.Flush();
	}

	template <class T>
	void WriteProperty(const field_id_t field_id, const char *tag, const T &value) {
	}

	template <class FUNC>
	void WriteList(const field_id_t field_id, const char *tag, idx_t count, FUNC func) {
	}

private:
	ChecksumWriter checksum_writer;
};

//===--------------------------------------------------------------------===//
// Write Entries
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteVersion() {
	// write the version marker
	// note that we explicitly do not checksum the version entry
}

void WriteAheadLog::WriteCheckpoint(MetaBlockPointer meta_block) {
}

//===--------------------------------------------------------------------===//
// CREATE TABLE
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteCreateTable(const TableCatalogEntry &entry) {
}

//===--------------------------------------------------------------------===//
// DROP TABLE
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteDropTable(const TableCatalogEntry &entry) {
}

//===--------------------------------------------------------------------===//
// CREATE SCHEMA
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteCreateSchema(const SchemaCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::CREATE_SCHEMA);
	serializer.WriteProperty(101, "schema", entry.name);
	serializer.End();
}

//===--------------------------------------------------------------------===//
// SEQUENCES
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteCreateSequence(const SequenceCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::CREATE_SEQUENCE);
	serializer.WriteProperty(101, "sequence", &entry);
	serializer.End();
}

void WriteAheadLog::WriteDropSequence(const SequenceCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::DROP_SEQUENCE);
	serializer.WriteProperty(101, "schema", entry.schema.name);
	serializer.WriteProperty(102, "name", entry.name);
	serializer.End();
}

void WriteAheadLog::WriteSequenceValue(SequenceValue val) {
	auto &sequence = *val.entry;
	WriteAheadLogSerializer serializer(*this, WALType::SEQUENCE_VALUE);
	serializer.WriteProperty(101, "schema", sequence.schema.name);
	serializer.WriteProperty(102, "name", sequence.name);
	serializer.WriteProperty(103, "usage_count", val.usage_count);
	serializer.WriteProperty(104, "counter", val.counter);
	serializer.End();
}

//===--------------------------------------------------------------------===//
// MACROS
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteCreateMacro(const ScalarMacroCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::CREATE_MACRO);
	serializer.WriteProperty(101, "macro", &entry);
	serializer.End();
}

void WriteAheadLog::WriteDropMacro(const ScalarMacroCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::DROP_MACRO);
	serializer.WriteProperty(101, "schema", entry.schema.name);
	serializer.WriteProperty(102, "name", entry.name);
	serializer.End();
}

void WriteAheadLog::WriteCreateTableMacro(const TableMacroCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::CREATE_TABLE_MACRO);
	serializer.WriteProperty(101, "table", &entry);
	serializer.End();
}

void WriteAheadLog::WriteDropTableMacro(const TableMacroCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::DROP_TABLE_MACRO);
	serializer.WriteProperty(101, "schema", entry.schema.name);
	serializer.WriteProperty(102, "name", entry.name);
	serializer.End();
}

//===--------------------------------------------------------------------===//
// Indexes
//===--------------------------------------------------------------------===//

void SerializeIndexToWAL(WriteAheadLogSerializer &serializer, Index &index,
                         const case_insensitive_map_t<Value> &options) {

	// We will never write an index to the WAL that is not bound
}

void WriteAheadLog::WriteCreateIndex(const IndexCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::CREATE_INDEX);
	serializer.WriteProperty(101, "index_catalog_entry", &entry);

	auto db_options = database.GetDatabase().config.options;
	auto v1_0_0_storage = db_options.serialization_compatibility.serialization_version < 3;
	case_insensitive_map_t<Value> options;
	if (!v1_0_0_storage) {
		options.emplace("v1_0_0_storage", v1_0_0_storage);
	}

	// now serialize the index data to the persistent storage and write the index metadata
	auto &duck_index_entry = entry.Cast<DuckIndexEntry>();
	serializer.End();
}

void WriteAheadLog::WriteDropIndex(const IndexCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::DROP_INDEX);
	serializer.WriteProperty(101, "schema", entry.schema.name);
	serializer.WriteProperty(102, "name", entry.name);
	serializer.End();
}

//===--------------------------------------------------------------------===//
// Custom Types
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteCreateType(const TypeCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::CREATE_TYPE);
	serializer.WriteProperty(101, "type", &entry);
	serializer.End();
}

void WriteAheadLog::WriteDropType(const TypeCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::DROP_TYPE);
	serializer.WriteProperty(101, "schema", entry.schema.name);
	serializer.WriteProperty(102, "name", entry.name);
	serializer.End();
}

//===--------------------------------------------------------------------===//
// VIEWS
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteCreateView(const ViewCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::CREATE_VIEW);
	serializer.WriteProperty(101, "view", &entry);
	serializer.End();
}

void WriteAheadLog::WriteDropView(const ViewCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::DROP_VIEW);
	serializer.WriteProperty(101, "schema", entry.schema.name);
	serializer.WriteProperty(102, "name", entry.name);
	serializer.End();
}

//===--------------------------------------------------------------------===//
// DROP SCHEMA
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteDropSchema(const SchemaCatalogEntry &entry) {
	WriteAheadLogSerializer serializer(*this, WALType::DROP_SCHEMA);
	serializer.WriteProperty(101, "schema", entry.name);
	serializer.End();
}

//===--------------------------------------------------------------------===//
// DATA
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteSetTable(const string &schema, const string &table) {
	WriteAheadLogSerializer serializer(*this, WALType::USE_TABLE);
	serializer.WriteProperty(101, "schema", schema);
	serializer.WriteProperty(102, "table", table);
	serializer.End();
}

void WriteAheadLog::WriteInsert(DataChunk &chunk) {
	D_ASSERT(chunk.size() > 0);
	chunk.Verify();

	WriteAheadLogSerializer serializer(*this, WALType::INSERT_TUPLE);
	serializer.WriteProperty(101, "chunk", chunk);
	serializer.End();
}

void WriteAheadLog::WriteRowGroupData(const PersistentCollectionData &data) {
	D_ASSERT(!data.row_group_data.empty());

	WriteAheadLogSerializer serializer(*this, WALType::ROW_GROUP_DATA);
	serializer.WriteProperty(101, "row_group_data", data);
	serializer.End();
}

void WriteAheadLog::WriteDelete(DataChunk &chunk) {
	D_ASSERT(chunk.size() > 0);
	D_ASSERT(chunk.ColumnCount() == 1 && chunk.data[0].GetType() == LogicalType::ROW_TYPE);
	chunk.Verify();

	WriteAheadLogSerializer serializer(*this, WALType::DELETE_TUPLE);
	serializer.WriteProperty(101, "chunk", chunk);
	serializer.End();
}

void WriteAheadLog::WriteUpdate(DataChunk &chunk, const vector<column_t> &column_indexes) {
	D_ASSERT(chunk.size() > 0);
	D_ASSERT(chunk.ColumnCount() == 2);
	D_ASSERT(chunk.data[1].GetType().id() == LogicalType::ROW_TYPE);
	chunk.Verify();

	WriteAheadLogSerializer serializer(*this, WALType::UPDATE_TUPLE);
	serializer.WriteProperty(101, "column_indexes", column_indexes);
	serializer.WriteProperty(102, "chunk", chunk);
	serializer.End();
}

//===--------------------------------------------------------------------===//
// Write ALTER Statement
//===--------------------------------------------------------------------===//
void WriteAheadLog::WriteAlter(const AlterInfo &info) {
	WriteAheadLogSerializer serializer(*this, WALType::ALTER_INFO);
	serializer.WriteProperty(101, "info", &info);
	serializer.End();
}

//===--------------------------------------------------------------------===//
// FLUSH
//===--------------------------------------------------------------------===//
void WriteAheadLog::Flush() {
}

} // namespace duckdb
