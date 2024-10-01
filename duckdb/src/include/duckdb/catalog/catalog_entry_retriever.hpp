#pragma once

#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/common/enums/catalog_type.hpp"
#include "duckdb/common/enums/on_entry_not_found.hpp"
#include "duckdb/common/string.hpp"
#include "duckdb/parser/query_error_context.hpp"

#include <functional>

namespace duckdb {

class ClientContext;
class Catalog;
class CatalogEntry;

using catalog_entry_callback_t = std::function<void(CatalogEntry &)>;

// Wraps the Catalog::GetEntry method
class CatalogEntryRetriever {
public:
	explicit CatalogEntryRetriever(ClientContext &context) : context(context) {
	}
	CatalogEntryRetriever(const CatalogEntryRetriever &other) : callback(other.callback), context(other.context) {
	}

public:
	optional_ptr<SchemaCatalogEntry> GetSchema(const string &catalog, const string &name,
	                                           OnEntryNotFound on_entry_not_found = OnEntryNotFound::THROW_EXCEPTION,
	                                           QueryErrorContext error_context = QueryErrorContext());

	void SetCallback(catalog_entry_callback_t callback) {
		this->callback = std::move(callback);
	}
	catalog_entry_callback_t GetCallback() {
		return callback;
	}

private:
	using catalog_entry_retrieve_func_t = std::function<optional_ptr<CatalogEntry>()>;
	optional_ptr<CatalogEntry> GetEntryInternal(const catalog_entry_retrieve_func_t &retriever) {
		return nullptr;
	}

private:
	//! (optional) callback, called on every succesful entry retrieval
	catalog_entry_callback_t callback = nullptr;
	ClientContext &context;
};

} // namespace duckdb
