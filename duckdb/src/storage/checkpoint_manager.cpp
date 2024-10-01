#include "duckdb/storage/checkpoint_manager.hpp"

#include "duckdb/catalog/catalog_entry/duck_index_entry.hpp"
#include "duckdb/catalog/catalog_entry/duck_table_entry.hpp"
#include "duckdb/catalog/catalog_entry/index_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/scalar_macro_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/sequence_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/type_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/view_catalog_entry.hpp"
#include "duckdb/catalog/duck_catalog.hpp"
#include "duckdb/common/serializer/binary_deserializer.hpp"
#include "duckdb/common/serializer/binary_serializer.hpp"
#include "duckdb/execution/index/art/art.hpp"
#include "duckdb/execution/index/unbound_index.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/config.hpp"
#include "duckdb/main/connection.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/parser/parsed_data/create_schema_info.hpp"
#include "duckdb/parser/parsed_data/create_view_info.hpp"
#include "duckdb/planner/binder.hpp"
#include "duckdb/planner/bound_tableref.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"
#include "duckdb/storage/block_manager.hpp"
#include "duckdb/storage/checkpoint/table_data_reader.hpp"
#include "duckdb/storage/checkpoint/table_data_writer.hpp"
#include "duckdb/storage/metadata/metadata_reader.hpp"
#include "duckdb/storage/table/column_checkpoint_state.hpp"
#include "duckdb/transaction/meta_transaction.hpp"
#include "duckdb/transaction/transaction_manager.hpp"

namespace duckdb {

void ReorderTableEntries(catalog_entry_vector_t &tables);

SingleFileCheckpointWriter::SingleFileCheckpointWriter(AttachedDatabase &db, BlockManager &block_manager,
                                                       CheckpointType checkpoint_type)
    : CheckpointWriter(db), partial_block_manager(block_manager, PartialBlockType::FULL_CHECKPOINT),
      checkpoint_type(checkpoint_type) {
}

BlockManager &SingleFileCheckpointWriter::GetBlockManager() {
	auto &storage_manager = db.GetStorageManager().Cast<SingleFileStorageManager>();
	return *storage_manager.block_manager;
}

MetadataWriter &SingleFileCheckpointWriter::GetMetadataWriter() {
	return *metadata_writer;
}

MetadataManager &SingleFileCheckpointWriter::GetMetadataManager() {
	return GetBlockManager().GetMetadataManager();
}

unique_ptr<TableDataWriter> SingleFileCheckpointWriter::GetTableDataWriter(TableCatalogEntry &table) {
	return make_uniq<SingleFileTableDataWriter>(*this, table, *table_metadata_writer);
}

static catalog_entry_vector_t GetCatalogEntries(vector<reference<SchemaCatalogEntry>> &schemas) {
	catalog_entry_vector_t entries;
	for (auto &schema_p : schemas) {
		auto &schema = schema_p.get();
		entries.push_back(schema);
		schema.Scan(CatalogType::TYPE_ENTRY, [&](CatalogEntry &entry) {
			if (entry.internal) {
				return;
			}
			entries.push_back(entry);
		});

		schema.Scan(CatalogType::SEQUENCE_ENTRY, [&](CatalogEntry &entry) {
			if (entry.internal) {
				return;
			}
			entries.push_back(entry);
		});

		catalog_entry_vector_t tables;
		vector<reference<ViewCatalogEntry>> views;
		schema.Scan(CatalogType::TABLE_ENTRY, [&](CatalogEntry &entry) {
			if (entry.internal) {
				return;
			}
			if (entry.type == CatalogType::TABLE_ENTRY) {
				tables.push_back(entry.Cast<TableCatalogEntry>());
			} else if (entry.type == CatalogType::VIEW_ENTRY) {
				views.push_back(entry.Cast<ViewCatalogEntry>());
			} else {
				throw NotImplementedException("Catalog type for entries");
			}
		});
		// Reorder tables because of foreign key constraint
		for (auto &table : tables) {
			entries.push_back(table.get());
		}
		for (auto &view : views) {
			entries.push_back(view.get());
		}

		schema.Scan(CatalogType::SCALAR_FUNCTION_ENTRY, [&](CatalogEntry &entry) {
			if (entry.internal) {
				return;
			}
			if (entry.type == CatalogType::MACRO_ENTRY) {
				entries.push_back(entry);
			}
		});

		schema.Scan(CatalogType::TABLE_FUNCTION_ENTRY, [&](CatalogEntry &entry) {
			if (entry.internal) {
				return;
			}
			if (entry.type == CatalogType::TABLE_MACRO_ENTRY) {
				entries.push_back(entry);
			}
		});

		schema.Scan(CatalogType::INDEX_ENTRY, [&](CatalogEntry &entry) {
			D_ASSERT(!entry.internal);
			entries.push_back(entry);
		});
	}
	return entries;
}

void SingleFileCheckpointWriter::CreateCheckpoint() {
	auto &config = DBConfig::Get(db);
	auto &storage_manager = db.GetStorageManager().Cast<SingleFileStorageManager>();
	if (storage_manager.InMemory()) {
		return;
	}
	// assert that the checkpoint manager hasn't been used before
	D_ASSERT(!metadata_writer);

	auto &block_manager = GetBlockManager();
	auto &metadata_manager = GetMetadataManager();

	//! Set up the writers for the checkpoints
	metadata_writer = make_uniq<MetadataWriter>(metadata_manager);
	table_metadata_writer = make_uniq<MetadataWriter>(metadata_manager);

	// get the id of the first meta block
	auto meta_block = metadata_writer->GetMetaBlockPointer();

	vector<reference<SchemaCatalogEntry>> schemas;
	// we scan the set of committed schemas
	auto &catalog = Catalog::GetCatalog(db).Cast<DuckCatalog>();
	// write the actual data into the database

	// Create a serializer to write the checkpoint data
	// The serialized format is roughly:
	/*
	    {
	        schemas: [
	            {
	                schema: <schema_info>,
	                custom_types: [ { type: <type_info> }, ... ],
	                sequences: [ { sequence: <sequence_info> }, ... ],
	                tables: [ { table: <table_info> }, ... ],
	                views: [ { view: <view_info> }, ... ],
	                macros: [ { macro: <macro_info> }, ... ],
	                table_macros: [ { table_macro: <table_macro_info> }, ... ],
	                indexes: [ { index: <index_info>, root_offset <block_ptr> }, ... ]
	            }
	        ]
	    }
	 */
}

void CheckpointReader::LoadCheckpoint(CatalogTransaction transaction, MetadataReader &reader) {
}

MetadataManager &SingleFileCheckpointReader::GetMetadataManager() {
	return storage.block_manager->GetMetadataManager();
}

void SingleFileCheckpointReader::LoadFromStorage() {
	auto &block_manager = *storage.block_manager;
	auto &metadata_manager = GetMetadataManager();
	MetaBlockPointer meta_block(block_manager.GetMetaBlock(), 0);
	if (!meta_block.IsValid()) {
		// storage is empty
		return;
	}
}

void CheckpointWriter::WriteEntry(CatalogEntry &entry, Serializer &serializer) {
	serializer.WriteProperty(99, "catalog_type", entry.type);

	switch (entry.type) {
	case CatalogType::SCHEMA_ENTRY: {
		auto &schema = entry.Cast<SchemaCatalogEntry>();
		WriteSchema(schema, serializer);
		break;
	}
	case CatalogType::TYPE_ENTRY: {
		auto &custom_type = entry.Cast<TypeCatalogEntry>();
		WriteType(custom_type, serializer);
		break;
	}
	case CatalogType::SEQUENCE_ENTRY: {
		auto &seq = entry.Cast<SequenceCatalogEntry>();
		WriteSequence(seq, serializer);
		break;
	}
	case CatalogType::TABLE_ENTRY: {
		auto &table = entry.Cast<TableCatalogEntry>();
		WriteTable(table, serializer);
		break;
	}
	case CatalogType::VIEW_ENTRY: {
		auto &view = entry.Cast<ViewCatalogEntry>();
		WriteView(view, serializer);
		break;
	}
	case CatalogType::MACRO_ENTRY: {
		auto &macro = entry.Cast<ScalarMacroCatalogEntry>();
		WriteMacro(macro, serializer);
		break;
	}
	case CatalogType::TABLE_MACRO_ENTRY: {
		auto &macro = entry.Cast<TableMacroCatalogEntry>();
		WriteTableMacro(macro, serializer);
		break;
	}
	case CatalogType::INDEX_ENTRY: {
		auto &index = entry.Cast<IndexCatalogEntry>();
		WriteIndex(index, serializer);
		break;
	}
	default:
		throw InternalException("Unrecognized catalog type in CheckpointWriter::WriteEntry");
	}
}

//===--------------------------------------------------------------------===//
// Schema
//===--------------------------------------------------------------------===//
void CheckpointWriter::WriteSchema(SchemaCatalogEntry &schema, Serializer &serializer) {
	// write the schema data
	serializer.WriteProperty(100, "schema", &schema);
}

void CheckpointReader::ReadEntry(CatalogTransaction transaction, Deserializer &deserializer) {
	auto type = deserializer.ReadProperty<CatalogType>(99, "type");

	switch (type) {
	case CatalogType::SCHEMA_ENTRY: {
		ReadSchema(transaction, deserializer);
		break;
	}
	case CatalogType::TYPE_ENTRY: {
		ReadType(transaction, deserializer);
		break;
	}
	case CatalogType::SEQUENCE_ENTRY: {
		ReadSequence(transaction, deserializer);
		break;
	}
	case CatalogType::TABLE_ENTRY: {
		ReadTable(transaction, deserializer);
		break;
	}
	case CatalogType::VIEW_ENTRY: {
		ReadView(transaction, deserializer);
		break;
	}
	case CatalogType::MACRO_ENTRY: {
		ReadMacro(transaction, deserializer);
		break;
	}
	case CatalogType::TABLE_MACRO_ENTRY: {
		ReadTableMacro(transaction, deserializer);
		break;
	}
	case CatalogType::INDEX_ENTRY: {
		ReadIndex(transaction, deserializer);
		break;
	}
	default:
		throw InternalException("Unrecognized catalog type in CheckpointWriter::WriteEntry");
	}
}

void CheckpointReader::ReadSchema(CatalogTransaction transaction, Deserializer &deserializer) {
	// Read the schema and create it in the catalog
}

//===--------------------------------------------------------------------===//
// Views
//===--------------------------------------------------------------------===//
void CheckpointWriter::WriteView(ViewCatalogEntry &view, Serializer &serializer) {
	serializer.WriteProperty(100, "view", &view);
}

void CheckpointReader::ReadView(CatalogTransaction transaction, Deserializer &deserializer) {
}

//===--------------------------------------------------------------------===//
// Sequences
//===--------------------------------------------------------------------===//
void CheckpointWriter::WriteSequence(SequenceCatalogEntry &seq, Serializer &serializer) {
	serializer.WriteProperty(100, "sequence", &seq);
}

void CheckpointReader::ReadSequence(CatalogTransaction transaction, Deserializer &deserializer) {
}

//===--------------------------------------------------------------------===//
// Indexes
//===--------------------------------------------------------------------===//
void CheckpointWriter::WriteIndex(IndexCatalogEntry &index_catalog_entry, Serializer &serializer) {
	// The index data is written as part of WriteTableData
	// Here, we serialize the index catalog entry

	// we need to keep the tag "index", even though it is slightly misleading
	serializer.WriteProperty(100, "index", &index_catalog_entry);
}

void CheckpointReader::ReadIndex(CatalogTransaction transaction, Deserializer &deserializer) {
	// we need to keep the tag "index", even though it is slightly misleading.
	auto create_info = deserializer.ReadProperty<unique_ptr<CreateInfo>>(100, "index");
	auto &info = create_info->Cast<CreateIndexInfo>();

	// also, we have to read the root_block_pointer, which will not be valid for newer storage versions.
	// This leads to different code paths in this function.
	auto root_block_pointer =
	    deserializer.ReadPropertyWithExplicitDefault<BlockPointer>(101, "root_block_pointer", BlockPointer());

	// create the index in the catalog

	// look for the table in the catalog
	auto &schema = catalog.GetSchema(transaction, create_info->schema);
}

//===--------------------------------------------------------------------===//
// Custom Types
//===--------------------------------------------------------------------===//
void CheckpointWriter::WriteType(TypeCatalogEntry &type, Serializer &serializer) {
	serializer.WriteProperty(100, "type", &type);
}

void CheckpointReader::ReadType(CatalogTransaction transaction, Deserializer &deserializer) {
}

//===--------------------------------------------------------------------===//
// Macro's
//===--------------------------------------------------------------------===//
void CheckpointWriter::WriteMacro(ScalarMacroCatalogEntry &macro, Serializer &serializer) {
	serializer.WriteProperty(100, "macro", &macro);
}

void CheckpointReader::ReadMacro(CatalogTransaction transaction, Deserializer &deserializer) {
}

void CheckpointWriter::WriteTableMacro(TableMacroCatalogEntry &macro, Serializer &serializer) {
	serializer.WriteProperty(100, "table_macro", &macro);
}

void CheckpointReader::ReadTableMacro(CatalogTransaction transaction, Deserializer &deserializer) {
}

//===--------------------------------------------------------------------===//
// Table Metadata
//===--------------------------------------------------------------------===//
void SingleFileCheckpointWriter::WriteTable(TableCatalogEntry &table, Serializer &serializer) {
	// Write the table metadata
	serializer.WriteProperty(100, "table", &table);

	// Write the table data
	auto table_lock = table.GetStorage().GetCheckpointLock();
	if (auto writer = GetTableDataWriter(table)) {
		writer->WriteTableData(serializer);
	}
	// flush any partial blocks BEFORE releasing the table lock
	// flushing partial blocks updates where data lives and is not thread-safe
	partial_block_manager.FlushPartialBlocks();
}

void CheckpointReader::ReadTable(CatalogTransaction transaction, Deserializer &deserializer) {
}

void CheckpointReader::ReadTableData(CatalogTransaction transaction, Deserializer &deserializer,
                                     BoundCreateTableInfo &bound_info) {

	// written in "SingleFileTableDataWriter::FinalizeTable"
	auto table_pointer = deserializer.ReadProperty<MetaBlockPointer>(101, "table_pointer");
	auto total_rows = deserializer.ReadProperty<idx_t>(102, "total_rows");

	// Cover reading old storage files.
	auto index_pointers = deserializer.ReadPropertyWithExplicitDefault<vector<BlockPointer>>(103, "index_pointers", {});
	// Cover reading new storage files.
	auto index_storage_infos =
	    deserializer.ReadPropertyWithExplicitDefault<vector<IndexStorageInfo>>(104, "index_storage_infos", {});

	if (!index_storage_infos.empty()) {
		bound_info.indexes = index_storage_infos;

	} else {
		// This is an old duckdb file containing index pointers and deprecated storage.
		for (idx_t i = 0; i < index_pointers.size(); i++) {
			// Deprecated storage is always true for old duckdb files.
			IndexStorageInfo index_storage_info;
			index_storage_info.root_block_ptr = index_pointers[i];
			bound_info.indexes.push_back(index_storage_info);
		}
	}

	// FIXME: icky downcast to get the underlying MetadataReader
	bound_info.data->total_rows = total_rows;
}

} // namespace duckdb
