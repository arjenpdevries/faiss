//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/aggregate/physical_hash_aggregate.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/operator/aggregate/distinct_aggregate_data.hpp"
#include "duckdb/execution/operator/aggregate/grouped_aggregate_data.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/execution/radix_partitioned_hashtable.hpp"
#include "duckdb/parser/group_by_node.hpp"
#include "duckdb/storage/data_table.hpp"

namespace duckdb {

class ClientContext;
class BufferManager;
class PhysicalHashAggregate;

struct HashAggregateGroupingData {
public:
	HashAggregateGroupingData(GroupingSet &grouping_set_p, const GroupedAggregateData &grouped_aggregate_data,
	                          unique_ptr<DistinctAggregateCollectionInfo> &info);

public:
	RadixPartitionedHashTable table_data;
	unique_ptr<DistinctAggregateData> distinct_data;

public:
	bool HasDistinct() const;
};

struct HashAggregateGroupingGlobalState {
public:
	HashAggregateGroupingGlobalState(const HashAggregateGroupingData &data, ClientContext &context);
	// Radix state of the GROUPING_SET ht
	unique_ptr<GlobalSinkState> table_state;
	// State of the DISTINCT aggregates of this GROUPING_SET
	unique_ptr<DistinctAggregateState> distinct_state;
};

struct HashAggregateGroupingLocalState {
public:
	HashAggregateGroupingLocalState(const PhysicalHashAggregate &op, const HashAggregateGroupingData &data,
	                                ExecutionContext &context);

public:
	// Radix state of the GROUPING_SET ht
	unique_ptr<LocalSinkState> table_state;
	// Local states of the DISTINCT aggregates hashtables
	vector<unique_ptr<LocalSinkState>> distinct_states;
};

} // namespace duckdb
