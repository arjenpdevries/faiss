//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/catalog/catalog_entry.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/atomic.hpp"
#include "duckdb/common/common.hpp"
#include "duckdb/common/enums/catalog_type.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/exception/catalog_exception.hpp"
#include "duckdb/common/optional_ptr.hpp"
#include "duckdb/common/types/value.hpp"

#include <memory>

namespace duckdb {
struct AlterInfo;
class Catalog;
class CatalogSet;
class ClientContext;
class SchemaCatalogEntry;
class Serializer;
class Deserializer;
class Value;

struct CatalogTransaction;
struct CreateInfo;

//! Abstract base class of an entry in the catalog
class CatalogEntry {
public:
	//! The oid of the entry
	idx_t oid;
	//! The type of this catalog entry
	CatalogType type;
	//! The name of the entry
	string name;
	//! Whether or not the object is deleted
	bool deleted;
	//! Whether or not the object is temporary and should not be added to the WAL
	bool temporary;
	//! Whether or not the entry is an internal entry (cannot be deleted, not dumped, etc)
	bool internal;
	//! Timestamp at which the catalog entry was created
	atomic<transaction_t> timestamp;
	//! (optional) comment on this entry
	Value comment;
	//! (optional) extra data associated with this entry
	unordered_map<string, string> tags;

public:
	virtual Catalog &ParentCatalog();
	virtual const Catalog &ParentCatalog() const;

	void Serialize(Serializer &serializer) const;
	static unique_ptr<CreateInfo> Deserialize(Deserializer &deserializer);

public:
	void SetChild(unique_ptr<CatalogEntry> child);
	unique_ptr<CatalogEntry> TakeChild();
	bool HasChild() const;
	CatalogEntry &Child();

public:
	template <class TARGET>
	TARGET &Cast() {
		DynamicCastCheck<TARGET>(this);
		return reinterpret_cast<TARGET &>(*this);
	}
	template <class TARGET>
	const TARGET &Cast() const {
		DynamicCastCheck<TARGET>(this);
		return reinterpret_cast<const TARGET &>(*this);
	}
};

class InCatalogEntry : public CatalogEntry {
public:
	InCatalogEntry(CatalogType type, Catalog &catalog, string name);
	~InCatalogEntry();

	//! The catalog the entry belongs to
	Catalog &catalog;

public:
	Catalog &ParentCatalog() override {
		return catalog;
	}
	const Catalog &ParentCatalog() const override {
		return catalog;
	}

	void Verify(Catalog &catalog);
};

} // namespace duckdb
