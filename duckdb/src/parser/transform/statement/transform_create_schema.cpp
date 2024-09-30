#include "duckdb/parser/parsed_data/create_schema_info.hpp"
#include "duckdb/parser/statement/create_statement.hpp"
#include "duckdb/parser/transformer.hpp"

namespace duckdb {

unique_ptr<CreateStatement> Transformer::TransformCreateSchema(duckdb_libpgquery::PGCreateSchemaStmt &stmt) {
	return nullptr;
}

} // namespace duckdb
