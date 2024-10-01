#include "duckdb/main/connection.hpp"

#include "duckdb.h"
#include "duckdb/common/types/column/column_data_collection.hpp"
#include "duckdb/function/table/read_csv.hpp"
#include "duckdb/main/appender.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/connection_manager.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/main/query_profiler.hpp"
#include "duckdb/parser/parser.hpp"
#include "duckdb/planner/logical_operator.hpp"

namespace duckdb {

Connection::Connection(DatabaseInstance &database)
    : context(make_shared_ptr<ClientContext>(database.shared_from_this())) {
	ConnectionManager::Get(database).AddConnection(*context);
#ifdef DEBUG
	EnableProfiling();
	context->config.emit_profiler_output = false;
#endif
}

Connection::Connection(DuckDB &database) : Connection(*database.instance) {
}

Connection::Connection(Connection &&other) noexcept {
	std::swap(context, other.context);
	std::swap(warning_cb, other.warning_cb);
}

Connection &Connection::operator=(Connection &&other) noexcept {
	std::swap(context, other.context);
	std::swap(warning_cb, other.warning_cb);
	return *this;
}

Connection::~Connection() {
	if (!context) {
		return;
	}
	ConnectionManager::Get(*context->db).RemoveConnection(*context);
}

string Connection::GetProfilingInformation(ProfilerPrintFormat format) {
	return "";
}

optional_ptr<ProfilingNode> Connection::GetProfilingTree() {
	auto &client_config = ClientConfig::GetConfig(*context);
	auto enable_profiler = client_config.enable_profiler;

	if (!enable_profiler) {
		throw Exception(ExceptionType::SETTINGS, "Profiling is not enabled for this connection");
	}
	return nullptr;
}

void Connection::Interrupt() {
	context->Interrupt();
}

void Connection::EnableProfiling() {
	context->EnableProfiling();
}

void Connection::DisableProfiling() {
	context->DisableProfiling();
}

void Connection::EnableQueryVerification() {
	ClientConfig::GetConfig(*context).query_verification_enabled = true;
}

void Connection::DisableQueryVerification() {
	ClientConfig::GetConfig(*context).query_verification_enabled = false;
}

void Connection::ForceParallelism() {
	ClientConfig::GetConfig(*context).verify_parallelism = true;
}

unique_ptr<QueryResult> Connection::SendQuery(const string &query) {
	return context->Query(query, true);
}

unique_ptr<MaterializedQueryResult> Connection::Query(const string &query) {
	return nullptr;
}

DUCKDB_API string Connection::GetSubstrait(const string &query) {
	return "protobuf.GetValueUnsafe<string_t>().GetString()";
}

DUCKDB_API unique_ptr<QueryResult> Connection::FromSubstrait(const string &proto) {
	vector<Value> params;
	params.emplace_back(Value::BLOB_RAW(proto));
	return nullptr;
}

DUCKDB_API string Connection::GetSubstraitJSON(const string &query) {
	return "protobuf.GetValueUnsafe<string_t>().GetString()";
}

DUCKDB_API unique_ptr<QueryResult> Connection::FromSubstraitJSON(const string &json) {
	vector<Value> params;
	params.emplace_back(json);
	return nullptr;
}

unique_ptr<MaterializedQueryResult> Connection::Query(unique_ptr<SQLStatement> statement) {
	auto result = context->Query(std::move(statement), false);
	D_ASSERT(result->type == QueryResultType::MATERIALIZED_RESULT);
	return unique_ptr_cast<QueryResult, MaterializedQueryResult>(std::move(result));
}

unique_ptr<PendingQueryResult> Connection::PendingQuery(const string &query, bool allow_stream_result) {
	return context->PendingQuery(query, allow_stream_result);
}

unique_ptr<PendingQueryResult> Connection::PendingQuery(unique_ptr<SQLStatement> statement, bool allow_stream_result) {
	return context->PendingQuery(std::move(statement), allow_stream_result);
}

unique_ptr<PreparedStatement> Connection::Prepare(const string &query) {
	return context->Prepare(query);
}

unique_ptr<PreparedStatement> Connection::Prepare(unique_ptr<SQLStatement> statement) {
	return context->Prepare(std::move(statement));
}

unique_ptr<QueryResult> Connection::QueryParamsRecursive(const string &query, vector<Value> &values) {
	auto statement = Prepare(query);
	if (statement->HasError()) {
		return make_uniq<MaterializedQueryResult>(statement->error);
	}
	return statement->Execute(values, false);
}

unique_ptr<TableDescription> Connection::TableInfo(const string &table_name) {
	return TableInfo(INVALID_SCHEMA, table_name);
}

unique_ptr<TableDescription> Connection::TableInfo(const string &schema_name, const string &table_name) {
	return context->TableInfo(schema_name, table_name);
}

vector<unique_ptr<SQLStatement>> Connection::ExtractStatements(const string &query) {
	return context->ParseStatements(query);
}

unique_ptr<LogicalOperator> Connection::ExtractPlan(const string &query) {
	return context->ExtractPlan(query);
}

void Connection::Append(TableDescription &description, DataChunk &chunk) {
	if (chunk.size() == 0) {
		return;
	}
	ColumnDataCollection collection(Allocator::Get(*context), chunk.GetTypes());
	collection.Append(chunk);
	Append(description, collection);
}

void Connection::Append(TableDescription &description, ColumnDataCollection &collection) {
	context->Append(description, collection);
}

unordered_set<string> Connection::GetTableNames(const string &query) {
	return context->GetTableNames(query);
}

void Connection::BeginTransaction() {
	auto result = Query("BEGIN TRANSACTION");
	if (result->HasError()) {
		result->ThrowError();
	}
}

void Connection::Commit() {
	auto result = Query("COMMIT");
	if (result->HasError()) {
		result->ThrowError();
	}
}

void Connection::Rollback() {
	auto result = Query("ROLLBACK");
	if (result->HasError()) {
		result->ThrowError();
	}
}

void Connection::SetAutoCommit(bool auto_commit) {
}

bool Connection::IsAutoCommit() {
	throw Exception(ExceptionType::SETTINGS, "Profiling is not enabled for this connection");
}
bool Connection::HasActiveTransaction() {
	throw Exception(ExceptionType::SETTINGS, "Profiling is not enabled for this connection");
}

} // namespace duckdb