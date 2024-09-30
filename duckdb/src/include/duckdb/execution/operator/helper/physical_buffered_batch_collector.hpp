//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/helper/physical_buffered_batch_collector.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/queue.hpp"
#include "duckdb/execution/operator/helper/physical_result_collector.hpp"

namespace duckdb {

class BufferedBatchCollectorLocalState : public LocalSinkState {
public:
	BufferedBatchCollectorLocalState();

public:
	idx_t current_batch = 0;
};

class PhysicalBufferedBatchCollector {
public:
	explicit PhysicalBufferedBatchCollector(PreparedStatementData &data);

public:
	unique_ptr<QueryResult> GetResult(GlobalSinkState &state);

public:
	// Sink interface
	SinkResultType Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const;
	SinkCombineResultType Combine(ExecutionContext &context, OperatorSinkCombineInput &input) const;
	SinkNextBatchType NextBatch(ExecutionContext &context, OperatorSinkNextBatchInput &input) const;

	unique_ptr<LocalSinkState> GetLocalSinkState(ExecutionContext &context) const;
	unique_ptr<GlobalSinkState> GetGlobalSinkState(ClientContext &context) const;

	bool RequiresBatchIndex() const {
		return true;
	}

	bool ParallelSink() const {
		return true;
	}

	bool IsStreaming() const {
		return true;
	}
};

} // namespace duckdb
