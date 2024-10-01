#include "faiss_extension.hpp"

#include "duckdb/common/shared_ptr.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/connection.hpp"
#include "duckdb/main/database.hpp"

#include <string>
#define DUCKDB_EXTENSION_MAIN

namespace duckdb {

// LoadInternal adds the faiss functions to the database
static void LoadInternal(DatabaseInstance &instance) {
}

void FaissExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}

std::string FaissExtension::Name() {
	return "faiss";
}

std::string FaissExtension::Version() const {
	return "0.9.0";
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void faiss_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::FaissExtension>();
}

DUCKDB_EXTENSION_API const char *faiss_version() {
	return NULL;
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
