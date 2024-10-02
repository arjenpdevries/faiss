//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/main/connection.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/enums/profiler_format.hpp"
#include "duckdb/common/serializer/buffered_file_writer.hpp"
#include "duckdb/common/winapi.hpp"
#include "duckdb/main/materialized_query_result.hpp"
#include "duckdb/main/pending_query_result.hpp"
#include "duckdb/main/prepared_statement.hpp"
#include "duckdb/main/profiling_node.hpp"
#include "duckdb/main/query_result.hpp"
#include "duckdb/main/relation.hpp"
#include "duckdb/main/stream_query_result.hpp"
#include "duckdb/main/table_description.hpp"
#include "duckdb/parser/sql_statement.hpp"

namespace duckdb {

class ColumnDataCollection;
class ClientContext;

class DatabaseInstance;
class DuckDB;
class LogicalOperator;
class SelectStatement;
struct CSVReaderOptions;

typedef void (*warning_callback_t)(std::string);

//! A connection to a database. This represents a (client) connection that can
//! be used to query the database.
class Connection {
public:
	DUCKDB_API explicit Connection(DuckDB &database);
	DUCKDB_API explicit Connection(DatabaseInstance &database);
	// disable copy constructors
	Connection(const Connection &other) = delete;
	Connection &operator=(const Connection &) = delete;
	//! enable move constructors
	DUCKDB_API Connection(Connection &&other) noexcept;
	DUCKDB_API Connection &operator=(Connection &&) noexcept;
	DUCKDB_API ~Connection();

	shared_ptr<ClientContext> context;
	warning_callback_t warning_cb;

public:
	//! Returns query profiling information for the current query
	DUCKDB_API string GetProfilingInformation(ProfilerPrintFormat format = ProfilerPrintFormat::QUERY_TREE);

	//! Returns the first node of the query profiling tree
	DUCKDB_API optional_ptr<ProfilingNode> GetProfilingTree();

	//! Interrupt execution of the current query
	DUCKDB_API void Interrupt();

	//! Enable query profiling
	DUCKDB_API void EnableProfiling();
	//! Disable query profiling
	DUCKDB_API void DisableProfiling();

	//! Enable aggressive verification/testing of queries, should only be used in testing
	DUCKDB_API void EnableQueryVerification();
	DUCKDB_API void DisableQueryVerification();
	//! Force parallel execution, even for smaller tables. Should only be used in testing.
	DUCKDB_API void ForceParallelism();

	//! Issues a query to the database and returns a QueryResult. This result can be either a StreamQueryResult or a
	//! MaterializedQueryResult. The result can be stepped through with calls to Fetch(). Note that there can only be
	//! one active StreamQueryResult per Connection object. Calling SendQuery() will invalidate any previously existing
	//! StreamQueryResult.
	DUCKDB_API unique_ptr<QueryResult> SendQuery(const string &query);
	//! Issues a query to the database and materializes the result (if necessary). Always returns a
	//! MaterializedQueryResult.
	DUCKDB_API unique_ptr<MaterializedQueryResult> Query(const string &query);
	//! Issues a query to the database and materializes the result (if necessary). Always returns a
	//! MaterializedQueryResult.
	DUCKDB_API unique_ptr<MaterializedQueryResult> Query(unique_ptr<SQLStatement> statement);
	// prepared statements
	template <typename... ARGS>
	unique_ptr<QueryResult> Query(const string &query, ARGS... args) {
		vector<Value> values;
		return QueryParamsRecursive(query, values, args...);
	}

	//! Issues a query to the database and returns a Pending Query Result. Note that "query" may only contain
	//! a single statement.
	DUCKDB_API unique_ptr<PendingQueryResult> PendingQuery(const string &query, bool allow_stream_result = false);
	//! Issues a query to the database and returns a Pending Query Result
	DUCKDB_API unique_ptr<PendingQueryResult> PendingQuery(unique_ptr<SQLStatement> statement,
	                                                       bool allow_stream_result = false);

	//! Prepare the specified query, returning a prepared statement object
	DUCKDB_API unique_ptr<PreparedStatement> Prepare(const string &query);
	//! Prepare the specified statement, returning a prepared statement object
	DUCKDB_API unique_ptr<PreparedStatement> Prepare(unique_ptr<SQLStatement> statement);

	//! Get the table info of a specific table (in the default schema), or nullptr if it cannot be found
	DUCKDB_API unique_ptr<TableDescription> TableInfo(const string &table_name);
	//! Get the table info of a specific table, or nullptr if it cannot be found
	DUCKDB_API unique_ptr<TableDescription> TableInfo(const string &schema_name, const string &table_name);

	//! Extract a set of SQL statements from a specific query
	DUCKDB_API vector<unique_ptr<SQLStatement>> ExtractStatements(const string &query);
	//! Extract the logical plan that corresponds to a query
	DUCKDB_API unique_ptr<LogicalOperator> ExtractPlan(const string &query);

	//! Appends a DataChunk to the specified table
	DUCKDB_API void Append(TableDescription &description, DataChunk &chunk);
	//! Appends a ColumnDataCollection to the specified table
	DUCKDB_API void Append(TableDescription &description, ColumnDataCollection &collection);

	//! Returns a substrait BLOB from a valid query
	DUCKDB_API string GetSubstrait(const string &query);
	//! Returns a Query Result from a substrait blob
	DUCKDB_API unique_ptr<QueryResult> FromSubstrait(const string &proto);
	//! Returns a substrait BLOB from a valid query
	DUCKDB_API string GetSubstraitJSON(const string &query);
	//! Returns a Query Result from a substrait JSON
	DUCKDB_API unique_ptr<QueryResult> FromSubstraitJSON(const string &json);
	DUCKDB_API void BeginTransaction();
	DUCKDB_API void Commit();
	DUCKDB_API void Rollback();
	DUCKDB_API void SetAutoCommit(bool auto_commit);
	DUCKDB_API bool IsAutoCommit();
	DUCKDB_API bool HasActiveTransaction();

	//! Fetch a list of table names that are required for a given query
	DUCKDB_API unordered_set<string> GetTableNames(const string &query);

	// NOLINTBEGIN
	template <typename TR, typename... ARGS>
	void CreateScalarFunction(const string &name, TR (*udf_func)(ARGS...)) {
	}

	template <typename TR, typename... ARGS>
	void CreateScalarFunction(const string &name, vector<LogicalType> args, LogicalType ret_type,
	                          TR (*udf_func)(ARGS...)) {
	}

	template <typename TR, typename... ARGS>
	void CreateVectorizedFunction(const string &name, scalar_function_t udf_func, LogicalType varargs) {
	}

	void CreateVectorizedFunction(const string &name, vector<LogicalType> args, LogicalType ret_type,
	                              scalar_function_t udf_func, LogicalType varargs) {
	}

	//------------------------------------- Aggreate Functions ----------------------------------------//
	template <typename UDF_OP, typename STATE, typename TR, typename TA>
	void CreateAggregateFunction(const string &name) {
	}

	template <typename UDF_OP, typename STATE, typename TR, typename TA, typename TB>
	void CreateAggregateFunction(const string &name) {
	}

	template <typename UDF_OP, typename STATE, typename TR, typename TA>
	void CreateAggregateFunction(const string &name, LogicalType ret_type, LogicalType input_type_a) {
	}

	template <typename UDF_OP, typename STATE, typename TR, typename TA, typename TB>
	void CreateAggregateFunction(const string &name, LogicalType ret_type, LogicalType input_type_a,
	                             LogicalType input_type_b) {
	}

private:
	unique_ptr<QueryResult> QueryParamsRecursive(const string &query, vector<Value> &values);

	template <typename T, typename... ARGS>
	unique_ptr<QueryResult> QueryParamsRecursive(const string &query, vector<Value> &values, T value, ARGS... args) {
		return QueryParamsRecursive(query, values, args...);
	}
};

} // namespace duckdb
