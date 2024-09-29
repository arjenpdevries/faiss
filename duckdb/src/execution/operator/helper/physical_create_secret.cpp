#include "duckdb/execution/operator/helper/physical_create_secret.hpp"

#include "duckdb/main/client_context.hpp"
#include "duckdb/main/database.hpp"

namespace duckdb {

SourceResultType PhysicalCreateSecret::GetData(ExecutionContext &context, DataChunk &chunk,
                                               OperatorSourceInput &input) const {
	auto &client = context.client;
	chunk.SetValue(0, 0, true);
	chunk.SetCardinality(1);

	return SourceResultType::FINISHED;
}

} // namespace duckdb
