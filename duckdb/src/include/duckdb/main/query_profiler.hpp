//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/main/query_profiler.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"
#include "duckdb/common/deque.hpp"
#include "duckdb/common/enums/explain_format.hpp"
#include "duckdb/common/enums/profiler_format.hpp"
#include "duckdb/common/pair.hpp"
#include "duckdb/common/profiler.hpp"
#include "duckdb/common/reference_map.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/common/unordered_map.hpp"
#include "duckdb/common/winapi.hpp"
#include "duckdb/execution/expression_executor_state.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/main/profiling_info.hpp"
#include "duckdb/main/profiling_node.hpp"

#include <stack>

namespace duckdb {
class ClientContext;
class ExpressionExecutor;
class ProfilingNode;
class PhysicalOperator;
class SQLStatement;

struct OperatorInformation {
	explicit OperatorInformation(double time_p = 0, idx_t elements_returned_p = 0, idx_t elements_scanned_p = 0,
	                             idx_t result_set_size_p = 0)
	    : time(time_p), elements_returned(elements_returned_p), result_set_size(result_set_size_p) {
	}

	double time;
	idx_t elements_returned;
	idx_t result_set_size;
	string name;

	void AddTime(double n_time) {
		time += n_time;
	}

	void AddReturnedElements(idx_t n_elements) {
		elements_returned += n_elements;
	}

	void AddResultSetSize(idx_t n_result_set_size) {
		result_set_size += n_result_set_size;
	}
};

//! The OperatorProfiler measures timings of individual operators
//! This class exists once for all operators and collects `OperatorInfo` for each operator
class OperatorProfiler {
	friend class QueryProfiler;

public:
	DUCKDB_API explicit OperatorProfiler(ClientContext &context);

	DUCKDB_API void StartOperator(optional_ptr<const PhysicalOperator> phys_op);
	DUCKDB_API void EndOperator(optional_ptr<DataChunk> chunk);

	//! Adds the timings in the OperatorProfiler (tree) to the QueryProfiler (tree).
	DUCKDB_API void Flush(const PhysicalOperator &phys_op);
	DUCKDB_API OperatorInformation &GetOperatorInfo(const PhysicalOperator &phys_op);

	~OperatorProfiler() {
	}

	ClientContext &context;

	bool HasOperatorSetting(const MetricsType &metric) const {
		return operator_settings.find(metric) != operator_settings.end();
	}

private:
	//! Whether or not the profiler is enabled
	bool enabled;
	//! Sub-settings for the operator profiler
	profiler_settings_t operator_settings;

	//! The timer used to time the execution time of the individual Physical Operators
	Profiler op;
	//! The stack of Physical Operators that are currently active
	optional_ptr<const PhysicalOperator> active_operator;
	//! A mapping of physical operators to recorded timings
	reference_map_t<const PhysicalOperator, OperatorInformation> timings;
};

struct QueryInfo {
	QueryInfo() : blocked_thread_time(0) {};
	string query_name;
	double blocked_thread_time;
};

} // namespace duckdb
