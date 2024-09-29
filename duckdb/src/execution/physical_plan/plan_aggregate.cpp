#include "duckdb/catalog/catalog_entry/aggregate_function_catalog_entry.hpp"
#include "duckdb/common/operator/subtract.hpp"
#include "duckdb/execution/operator/aggregate/physical_hash_aggregate.hpp"
#include "duckdb/execution/operator/aggregate/physical_perfecthash_aggregate.hpp"
#include "duckdb/execution/operator/aggregate/physical_ungrouped_aggregate.hpp"
#include "duckdb/execution/operator/projection/physical_projection.hpp"
#include "duckdb/execution/physical_plan_generator.hpp"
#include "duckdb/function/function_binder.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/parser/expression/comparison_expression.hpp"
#include "duckdb/planner/expression/bound_aggregate_expression.hpp"
#include "duckdb/planner/expression/bound_reference_expression.hpp"
#include "duckdb/planner/operator/logical_aggregate.hpp"

namespace duckdb {

static uint32_t RequiredBitsForValue(uint32_t n) {
	idx_t required_bits = 0;
	while (n > 0) {
		n >>= 1;
		required_bits++;
	}
	return UnsafeNumericCast<uint32_t>(required_bits);
}

template <class T>
hugeint_t GetRangeHugeint(const BaseStatistics &nstats) {
	return Hugeint::Convert(0);
}

static bool CanUsePerfectHashAggregate(ClientContext &context, LogicalAggregate &op, vector<idx_t> &bits_per_group) {
	if (op.grouping_sets.size() > 1 || !op.grouping_functions.empty()) {
		return false;
	}
	idx_t perfect_hash_bits = 0;
	for (idx_t group_idx = 0; group_idx < op.groups.size(); group_idx++) {
		auto &group = op.groups[group_idx];

		switch (group->return_type.InternalType()) {
		case PhysicalType::INT8:
		case PhysicalType::INT16:
		case PhysicalType::INT32:
		case PhysicalType::INT64:
		case PhysicalType::UINT8:
		case PhysicalType::UINT16:
		case PhysicalType::UINT32:
		case PhysicalType::UINT64:
			break;
		default:
			// we only support simple integer types for perfect hashing
			return false;
		}
		// check if the group has stats available
		auto &group_type = group->return_type;
	}
	for (auto &expression : op.expressions) {
		auto &aggregate = expression->Cast<BoundAggregateExpression>();
		if (aggregate.IsDistinct() || !aggregate.function.combine) {
			// distinct aggregates are not supported in perfect hash aggregates
			return false;
		}
	}
	return true;
}

unique_ptr<PhysicalOperator> PhysicalPlanGenerator::CreatePlan(LogicalAggregate &op) {
	unique_ptr<PhysicalOperator> groupby;
	D_ASSERT(op.children.size() == 1);

	auto plan = CreatePlan(*op.children[0]);

	plan = ExtractAggregateExpressions(std::move(plan), op.expressions, op.groups);

	if (op.groups.empty() && op.grouping_sets.size() <= 1) {
		// no groups, check if we can use a simple aggregation
		// special case: aggregate entire columns together
		bool use_simple_aggregation = true;
		for (auto &expression : op.expressions) {
			auto &aggregate = expression->Cast<BoundAggregateExpression>();
			if (!aggregate.function.simple_update) {
				// unsupported aggregate for simple aggregation: use hash aggregation
				use_simple_aggregation = false;
				break;
			}
		}
		if (use_simple_aggregation) {
			groupby = make_uniq_base<PhysicalOperator, PhysicalUngroupedAggregate>(op.types, std::move(op.expressions),
			                                                                       op.estimated_cardinality);
		} else {
			groupby = make_uniq_base<PhysicalOperator, PhysicalHashAggregate>(
			    context, op.types, std::move(op.expressions), op.estimated_cardinality);
		}
	} else {
		// groups! create a GROUP BY aggregator
		// use a perfect hash aggregate if possible
		vector<idx_t> required_bits;
	}
	groupby->children.push_back(std::move(plan));
	return groupby;
}

unique_ptr<PhysicalOperator>
PhysicalPlanGenerator::ExtractAggregateExpressions(unique_ptr<PhysicalOperator> child,
                                                   vector<unique_ptr<Expression>> &aggregates,
                                                   vector<unique_ptr<Expression>> &groups) {
	vector<unique_ptr<Expression>> expressions;
	vector<LogicalType> types;

	// bind sorted aggregates
	for (auto &aggr : aggregates) {
		auto &bound_aggr = aggr->Cast<BoundAggregateExpression>();
		if (bound_aggr.order_bys) {
			// sorted aggregate!
			FunctionBinder::BindSortedAggregate(context, bound_aggr, groups);
		}
	}
	for (auto &group : groups) {
		auto ref = make_uniq<BoundReferenceExpression>(group->return_type, expressions.size());
		types.push_back(group->return_type);
		expressions.push_back(std::move(group));
		group = std::move(ref);
	}
	for (auto &aggr : aggregates) {
		auto &bound_aggr = aggr->Cast<BoundAggregateExpression>();
		for (auto &child : bound_aggr.children) {
			auto ref = make_uniq<BoundReferenceExpression>(child->return_type, expressions.size());
			types.push_back(child->return_type);
			expressions.push_back(std::move(child));
			child = std::move(ref);
		}
		if (bound_aggr.filter) {
			auto &filter = bound_aggr.filter;
			auto ref = make_uniq<BoundReferenceExpression>(filter->return_type, expressions.size());
			types.push_back(filter->return_type);
			expressions.push_back(std::move(filter));
			bound_aggr.filter = std::move(ref);
		}
	}
	if (expressions.empty()) {
		return child;
	}
	auto projection =
	    make_uniq<PhysicalProjection>(std::move(types), std::move(expressions), child->estimated_cardinality);
	projection->children.push_back(std::move(child));
	return std::move(projection);
}

} // namespace duckdb
