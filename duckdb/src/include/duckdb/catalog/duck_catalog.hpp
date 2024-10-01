//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/catalog/dcatalog.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/catalog/catalog.hpp"

namespace duckdb {

//! The Catalog object represents the catalog of the database.
class DuckCatalog : public Catalog {
public:
	explicit DuckCatalog(AttachedDatabase &db);
	~DuckCatalog() override;

public:
	bool IsDuckCatalog() override;
	void Initialize(bool load_builtin) override;
	string GetCatalogType() override {
		return "duckdb";
	}

	mutex &GetWriteLock() {
		return write_lock;
	}

public:
	DUCKDB_API optional_ptr<CatalogEntry> CreateSchema(CatalogTransaction transaction);
	DUCKDB_API void ScanSchemas(ClientContext &context, std::function<void(SchemaCatalogEntry &)> callback) override;
	DUCKDB_API void ScanSchemas(std::function<void(SchemaCatalogEntry &)> callback);

	DUCKDB_API optional_ptr<SchemaCatalogEntry>
	GetSchema(CatalogTransaction transaction, const string &schema_name, OnEntryNotFound if_not_found,
	          QueryErrorContext error_context = QueryErrorContext()) override;

	DatabaseSize GetDatabaseSize(ClientContext &context) override;
	vector<MetadataBlockInfo> GetMetadataInfo(ClientContext &context) override;

	DUCKDB_API bool InMemory() override;
	DUCKDB_API string GetDBPath() override;

	DUCKDB_API optional_idx GetCatalogVersion(ClientContext &context) override;

private:
	DUCKDB_API void DropSchema(CatalogTransaction transaction, DropInfo &info);
	DUCKDB_API void DropSchema(ClientContext &context, DropInfo &info) override;
	optional_ptr<CatalogEntry> CreateSchemaInternal(CatalogTransaction transaction);
	void Verify() override;

private:
	//! Write lock for the catalog
	mutex write_lock;
};

} // namespace duckdb
