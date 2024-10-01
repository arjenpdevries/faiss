#include "duckdb/catalog/duck_catalog.hpp"

#include "duckdb/catalog/catalog_entry/duck_schema_entry.hpp"
#include "duckdb/catalog/default/default_schemas.hpp"
#include "duckdb/catalog/dependency_manager.hpp"
#include "duckdb/function/built_in_functions.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/parser/parsed_data/create_schema_info.hpp"
#include "duckdb/parser/parsed_data/drop_info.hpp"
#include "duckdb/storage/storage_manager.hpp"
#include "duckdb/transaction/duck_transaction_manager.hpp"
#ifndef DISABLE_CORE_FUNCTIONS_EXTENSION
#include "duckdb/core_functions/core_functions.hpp"
#endif

namespace duckdb {

DuckCatalog::DuckCatalog(AttachedDatabase &db) : Catalog(db), schemas(make_uniq<CatalogSet>(*this)) {
}

DuckCatalog::~DuckCatalog() {
}

void DuckCatalog::Initialize(bool load_builtin) {
	// first initialize the base system catalogs
	// these are never written to the WAL
	// we start these at 1 because deleted entries default to 0
	auto data = CatalogTransaction::GetSystemTransaction(GetDatabase());

	// create the default schema

	Verify();
}

bool DuckCatalog::IsDuckCatalog() {
	return true;
}

//===--------------------------------------------------------------------===//
// Schema
//===--------------------------------------------------------------------===//
optional_ptr<CatalogEntry> DuckCatalog::CreateSchemaInternal(CatalogTransaction transaction) {
	return nullptr;
}

optional_ptr<CatalogEntry> DuckCatalog::CreateSchema(CatalogTransaction transaction) {
	return nullptr;
}

void DuckCatalog::DropSchema(CatalogTransaction transaction, DropInfo &info) {
	D_ASSERT(!info.name.empty());
	if (!schemas->DropEntry(transaction, info.name, info.cascade)) {
		if (info.if_not_found == OnEntryNotFound::THROW_EXCEPTION) {
			throw CatalogException::MissingEntry(CatalogType::SCHEMA_ENTRY, info.name, string());
		}
	}
}

void DuckCatalog::DropSchema(ClientContext &context, DropInfo &info) {
	DropSchema(GetCatalogTransaction(context), info);
}

void DuckCatalog::ScanSchemas(ClientContext &context, std::function<void(SchemaCatalogEntry &)> callback) {
	schemas->Scan(GetCatalogTransaction(context),
	              [&](CatalogEntry &entry) { callback(entry.Cast<SchemaCatalogEntry>()); });
}

void DuckCatalog::ScanSchemas(std::function<void(SchemaCatalogEntry &)> callback) {
	schemas->Scan([&](CatalogEntry &entry) { callback(entry.Cast<SchemaCatalogEntry>()); });
}

optional_ptr<SchemaCatalogEntry> DuckCatalog::GetSchema(CatalogTransaction transaction, const string &schema_name,
                                                        OnEntryNotFound if_not_found, QueryErrorContext error_context) {
	D_ASSERT(!schema_name.empty());
	auto entry = schemas->GetEntry(transaction, schema_name);
	if (!entry) {
		if (if_not_found == OnEntryNotFound::THROW_EXCEPTION) {
			throw CatalogException(error_context, "Schema with name %s does not exist!", schema_name);
		}
		return nullptr;
	}
	return &entry->Cast<SchemaCatalogEntry>();
}

DatabaseSize DuckCatalog::GetDatabaseSize(ClientContext &context) {
	auto &transaction = DuckTransactionManager::Get(db);
	auto lock = transaction.SharedCheckpointLock();
	return db.GetStorageManager().GetDatabaseSize();
}

vector<MetadataBlockInfo> DuckCatalog::GetMetadataInfo(ClientContext &context) {
	auto &transaction = DuckTransactionManager::Get(db);
	auto lock = transaction.SharedCheckpointLock();
	return db.GetStorageManager().GetMetadataInfo();
}

bool DuckCatalog::InMemory() {
	return db.GetStorageManager().InMemory();
}

string DuckCatalog::GetDBPath() {
	return db.GetStorageManager().GetDBPath();
}

void DuckCatalog::Verify() {
#ifdef DEBUG
	Catalog::Verify();
	schemas->Verify(*this);
#endif
}

optional_idx DuckCatalog::GetCatalogVersion(ClientContext &context) {
	auto &transaction_manager = DuckTransactionManager::Get(db);
	auto transaction = GetCatalogTransaction(context);
	D_ASSERT(transaction.transaction);
	return transaction_manager.GetCatalogVersion(*transaction.transaction);
}

} // namespace duckdb
