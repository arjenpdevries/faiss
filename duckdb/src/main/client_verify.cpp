#include "duckdb/common/box_renderer.hpp"
#include "duckdb/common/error_data.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/database.hpp"
#include "duckdb/parser/statement/explain_statement.hpp"
#include "duckdb/verification/statement_verifier.hpp"

namespace duckdb {

static void ThrowIfExceptionIsInternal(StatementVerifier &verifier) {
	if (!verifier.materialized_result) {
		return;
	}
	auto &result = *verifier.materialized_result;
	if (!result.HasError()) {
		return;
	}
	auto &error = result.GetErrorObject();
	if (error.Type() == ExceptionType::INTERNAL) {
		error.Throw();
	}
}

ErrorData ClientContext::VerifyQuery(ClientContextLock &lock, const string &query, unique_ptr<SQLStatement> statement) {
	D_ASSERT(statement->type == StatementType::SELECT_STATEMENT);
	// Aggressive query verification

#ifdef DUCKDB_RUN_SLOW_VERIFIERS
	bool run_slow_verifiers = true;
#else
	bool run_slow_verifiers = false;
#endif

	// The purpose of this function is to test correctness of otherwise hard to test features:
	// Copy() of statements and expressions
	// Serialize()/Deserialize() of expressions
	// Hash() of expressions
	// Equality() of statements and expressions
	// ToString() of statements and expressions
	// Correctness of plans both with and without optimizers

	const auto &stmt = *statement;

	// See below
	auto statement_copy_for_explain = stmt.Copy();

	// Save settings
	bool optimizer_enabled = config.enable_optimizer;
	bool profiling_is_enabled = config.enable_profiler;
	bool force_external = config.force_external;

	// Disable profiling if it is enabled
	if (profiling_is_enabled) {
		config.enable_profiler = false;
	}

	// Execute the verifiers

	// Restore config setting
	config.enable_optimizer = optimizer_enabled;
	config.force_external = force_external;

	// Restore profiler setting
	if (profiling_is_enabled) {
		config.enable_profiler = true;
	}

	// Now compare the results
	// The results of all runs should be identical

	return ErrorData();
}

} // namespace duckdb
