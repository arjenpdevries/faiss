//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/physical_plan_generator.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/catalog/dependency_list.hpp"
#include "duckdb/common/common.hpp"
#include "duckdb/common/unordered_map.hpp"
#include "duckdb/common/unordered_set.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/planner/joinside.hpp"
#include "duckdb/planner/logical_operator.hpp"
#include "duckdb/planner/logical_tokens.hpp"

namespace duckdb {
class ClientContext;
class ColumnDataCollection;

//! The physical plan generator generates a physical execution plan from a
//! logical query plan
class PhysicalPlanGenerator {
public:
	explicit PhysicalPlanGenerator(ClientContext &context);
	~PhysicalPlanGenerator();

	LogicalDependencyList dependencies;
	//! Recursive CTEs require at least one ChunkScan, referencing the working_table.
	//! This data structure is used to establish it.
	unordered_map<idx_t, shared_ptr<ColumnDataCollection>> recursive_cte_tables;
	//! Materialized CTE ids must be collected.
	unordered_map<idx_t, vector<const_reference<PhysicalOperator>>> materialized_ctes;

public:
	//! Creates a plan from the logical operator. This involves resolving column bindings and generating physical
	//! operator nodes.
	unique_ptr<PhysicalOperator> CreatePlan(unique_ptr<LogicalOperator> logical);

	//! Whether or not we can (or should) use a batch-index based operator for executing the given sink
	static bool UseBatchIndex(ClientContext &context, PhysicalOperator &plan);
	//! Whether or not we should preserve insertion order for executing the given sink
	static bool PreserveInsertionOrder(ClientContext &context, PhysicalOperator &plan);

	static bool HasEquality(vector<JoinCondition> &conds, idx_t &range_count);

private:
	bool PreserveInsertionOrder(PhysicalOperator &plan);
	bool UseBatchIndex(PhysicalOperator &plan);

public:
	idx_t delim_index = 0;

private:
	ClientContext &context;
};
} // namespace duckdb
