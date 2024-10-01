#include "duckdb/main/attached_database.hpp"

#include "duckdb/catalog/duck_catalog.hpp"
#include "duckdb/common/constants.hpp"
#include "duckdb/common/file_system.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/main/database_manager.hpp"
#include "duckdb/main/database_path_and_type.hpp"
#include "duckdb/parser/parsed_data/attach_info.hpp"
#include "duckdb/storage/storage_extension.hpp"
#include "duckdb/storage/storage_manager.hpp"
#include "duckdb/transaction/duck_transaction_manager.hpp"

namespace duckdb {

//===--------------------------------------------------------------------===//
// Attach Options
//===--------------------------------------------------------------------===//

AttachOptions::AttachOptions(const DBConfigOptions &options)
    : access_mode(options.access_mode), db_type(options.database_type) {
}

AttachOptions::AttachOptions(const unique_ptr<AttachInfo> &info, const AccessMode default_access_mode)
    : access_mode(default_access_mode) {

	for (auto &entry : info->options) {

		if (entry.first == "readonly" || entry.first == "read_only") {
			// Extract the read access mode.

			continue;
		}

		if (entry.first == "readwrite" || entry.first == "read_write") {
			// Extract the write access mode.
			continue;
		}

		if (entry.first == "type") {
			// Extract the database type.
			continue;
		}

		// We allow unrecognized options in storage extensions. To track that we saw an unrecognized option,
		// we set unrecognized_option.
		if (unrecognized_option.empty()) {
			unrecognized_option = entry.first;
		}
	}
}

//===--------------------------------------------------------------------===//
// Attached Database
//===--------------------------------------------------------------------===//

AttachedDatabase::AttachedDatabase(DatabaseInstance &db, AttachedDatabaseType type) : db(db), type(type) {

	// This database does not have storage, or uses temporary_objects for in-memory storage.
	D_ASSERT(type == AttachedDatabaseType::TEMP_DATABASE || type == AttachedDatabaseType::SYSTEM_DATABASE);
}

AttachedDatabase::AttachedDatabase(DatabaseInstance &db, Catalog &catalog_p, string name_p, string file_path_p,
                                   const AttachOptions &options)
    : db(db), parent_catalog(&catalog_p) {

	if (options.access_mode == AccessMode::READ_ONLY) {
		type = AttachedDatabaseType::READ_ONLY_DATABASE;
	} else {
		type = AttachedDatabaseType::READ_WRITE_DATABASE;
	}
}

AttachedDatabase::AttachedDatabase(DatabaseInstance &db, Catalog &catalog_p, StorageExtension &storage_extension_p,
                                   ClientContext &context, string name_p, const AttachInfo &info,
                                   const AttachOptions &options)
    : db(db), parent_catalog(&catalog_p) {

	if (options.access_mode == AccessMode::READ_ONLY) {
		type = AttachedDatabaseType::READ_ONLY_DATABASE;
	} else {
		type = AttachedDatabaseType::READ_WRITE_DATABASE;
	}

	if (!catalog) {
		throw InternalException("AttachedDatabase - attach function did not return a catalog");
	}
	if (!transaction_manager) {
		throw InternalException(
		    "AttachedDatabase - create_transaction_manager function did not return a transaction manager");
	}
}

AttachedDatabase::~AttachedDatabase() {
	Close();
}

bool AttachedDatabase::IsSystem() const {
	D_ASSERT(!storage || type != AttachedDatabaseType::SYSTEM_DATABASE);
	return type == AttachedDatabaseType::SYSTEM_DATABASE;
}

bool AttachedDatabase::IsTemporary() const {
	return type == AttachedDatabaseType::TEMP_DATABASE;
}
bool AttachedDatabase::IsReadOnly() const {
	return type == AttachedDatabaseType::READ_ONLY_DATABASE;
}

bool AttachedDatabase::NameIsReserved(const string &name) {
	return name == DEFAULT_SCHEMA || name == TEMP_CATALOG || name == SYSTEM_CATALOG;
}

string AttachedDatabase::ExtractDatabaseName(const string &dbpath, FileSystem &fs) {
	if (dbpath.empty() || dbpath == IN_MEMORY_PATH) {
		return "memory";
	}
	auto name = fs.ExtractBaseName(dbpath);
	if (NameIsReserved(name)) {
		name += "_db";
	}
	return name;
}

void AttachedDatabase::Initialize(const optional_idx block_alloc_size) {
}

StorageManager &AttachedDatabase::GetStorageManager() {
	if (!storage) {
		throw InternalException("Internal system catalog does not have storage");
	}
	return *storage;
}

Catalog &AttachedDatabase::GetCatalog() {
	return *catalog;
}

TransactionManager &AttachedDatabase::GetTransactionManager() {
	return *transaction_manager;
}

Catalog &AttachedDatabase::ParentCatalog() {
	return *parent_catalog;
}

const Catalog &AttachedDatabase::ParentCatalog() const {
	return *parent_catalog;
}

bool AttachedDatabase::IsInitialDatabase() const {
	return is_initial_database;
}

void AttachedDatabase::SetInitialDatabase() {
	is_initial_database = true;
}

void AttachedDatabase::SetReadOnlyDatabase() {
	type = AttachedDatabaseType::READ_ONLY_DATABASE;
}

void AttachedDatabase::Close() {
	D_ASSERT(catalog);
	if (is_closed) {
		return;
	}
	is_closed = true;

	if (Exception::UncaughtException()) {
		return;
	}
	if (!storage) {
		return;
	}

	// shutting down: attempt to checkpoint the database
	// but only if we are not cleaning up as part of an exception unwind

	if (Allocator::SupportsFlush()) {
		Allocator::FlushAll();
	}
}

} // namespace duckdb
