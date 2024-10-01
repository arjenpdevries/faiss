#include "duckdb/function/function_set.hpp"
#include "duckdb/function/table/range.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/database_manager.hpp"
#include "duckdb/storage/storage_manager.hpp"
#include "duckdb/transaction/transaction_manager.hpp"

namespace duckdb {

struct CheckpointBindData : public FunctionData {
	explicit CheckpointBindData(optional_ptr<AttachedDatabase> db) {
	}

public:
	unique_ptr<FunctionData> Copy() const override {
		return nullptr;
	}

	bool Equals(const FunctionData &other_p) const override {
		return false;
	}
};

static unique_ptr<FunctionData> CheckpointBind(ClientContext &context, TableFunctionBindInput &input,
                                               vector<LogicalType> &return_types, vector<string> &names) {
	return_types.emplace_back(LogicalType::BOOLEAN);
	names.emplace_back("Success");

	return nullptr;
}

template <bool FORCE>
static void TemplatedCheckpointFunction(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
}

void CheckpointFunction::RegisterFunction(BuiltinFunctions &set) {
	TableFunctionSet checkpoint("checkpoint");
	checkpoint.AddFunction(TableFunction({}, TemplatedCheckpointFunction<false>, CheckpointBind));
	checkpoint.AddFunction(TableFunction({LogicalType::VARCHAR}, TemplatedCheckpointFunction<false>, CheckpointBind));
	set.AddFunction(checkpoint);

	TableFunctionSet force_checkpoint("force_checkpoint");
	force_checkpoint.AddFunction(TableFunction({}, TemplatedCheckpointFunction<true>, CheckpointBind));
	force_checkpoint.AddFunction(
	    TableFunction({LogicalType::VARCHAR}, TemplatedCheckpointFunction<true>, CheckpointBind));
	set.AddFunction(force_checkpoint);
}

} // namespace duckdb
