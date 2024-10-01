//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/parser/statement/extension_statement.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/parser/parser_extension.hpp"
#include "duckdb/parser/sql_statement.hpp"

namespace duckdb {

class ExtensionStatement : public SQLStatement {
public:
	static constexpr const StatementType TYPE = StatementType::EXTENSION_STATEMENT;

public:
	//! The ParserExtension this statement was generated from
	ParserExtension extension;
	//! The parse data for this specific statement
	unique_ptr<ParserExtensionParseData> parse_data;

public:
	unique_ptr<SQLStatement> Copy() const override;
	string ToString() const override;
};

} // namespace duckdb
