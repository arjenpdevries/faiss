#include "duckdb/execution/operator/helper/physical_vacuum.hpp"

#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/planner/operator/logical_get.hpp"
#include "duckdb/storage/data_table.hpp"
#include "duckdb/storage/statistics/distinct_statistics.hpp"

namespace duckdb {

PhysicalVacuum::PhysicalVacuum(unique_ptr<VacuumInfo> info_p, optional_ptr<TableCatalogEntry> table,
                               unordered_map<idx_t, idx_t> column_id_map, idx_t estimated_cardinality)
    : PhysicalOperator(PhysicalOperatorType::VACUUM, {LogicalType::BOOLEAN}, estimated_cardinality),
      info(std::move(info_p)), table(table), column_id_map(std::move(column_id_map)) {
}

class VacuumLocalSinkState : public LocalSinkState {
public:
	explicit VacuumLocalSinkState(VacuumInfo &info, optional_ptr<TableCatalogEntry> table) {
		for (const auto &column_name : info.columns) {
			auto &column = table->GetColumn(column_name);
		}
	};
};

unique_ptr<LocalSinkState> PhysicalVacuum::GetLocalSinkState(ExecutionContext &context) const {
	return make_uniq<VacuumLocalSinkState>(*info, table);
}

class VacuumGlobalSinkState : public GlobalSinkState {
public:
	explicit VacuumGlobalSinkState(VacuumInfo &info, optional_ptr<TableCatalogEntry> table) {
		for (const auto &column_name : info.columns) {
			auto &column = table->GetColumn(column_name);
		}
	};

	mutex stats_lock;
};

unique_ptr<GlobalSinkState> PhysicalVacuum::GetGlobalSinkState(ClientContext &context) const {
	return make_uniq<VacuumGlobalSinkState>(*info, table);
}

SinkResultType PhysicalVacuum::Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const {
	auto &lstate = input.local_state.Cast<VacuumLocalSinkState>();

	return SinkResultType::NEED_MORE_INPUT;
}

SinkCombineResultType PhysicalVacuum::Combine(ExecutionContext &context, OperatorSinkCombineInput &input) const {
	auto &g_state = input.global_state.Cast<VacuumGlobalSinkState>();
	auto &l_state = input.local_state.Cast<VacuumLocalSinkState>();

	lock_guard<mutex> lock(g_state.stats_lock);

	return SinkCombineResultType::FINISHED;
}

SinkFinalizeType PhysicalVacuum::Finalize(Pipeline &pipeline, Event &event, ClientContext &context,
                                          OperatorSinkFinalizeInput &input) const {
	auto &sink = input.global_state.Cast<VacuumGlobalSinkState>();

	auto tbl = table;

	return SinkFinalizeType::READY;
}

SourceResultType PhysicalVacuum::GetData(ExecutionContext &context, DataChunk &chunk,
                                         OperatorSourceInput &input) const {
	// NOP
	return SourceResultType::FINISHED;
}

} // namespace duckdb
