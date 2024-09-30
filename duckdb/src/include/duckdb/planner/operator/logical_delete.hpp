//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/planner/operator/logical_delete.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/planner/bound_constraint.hpp"
#include "duckdb/planner/logical_operator.hpp"

namespace duckdb {
class TableCatalogEntry;

class LogicalDelete : public LogicalOperator {
public:
	static constexpr const LogicalOperatorType TYPE = LogicalOperatorType::LOGICAL_DELETE;

public:
	explicit LogicalDelete(TableCatalogEntry &table, idx_t table_index);

	TableCatalogEntry &table;
	idx_t table_index;
	bool return_chunk;
	vector<unique_ptr<BoundConstraint>> bound_constraints;

public:
	idx_t EstimateCardinality(ClientContext &context) override;
	vector<idx_t> GetTableIndex() const override;
	string GetName() const override;

protected:
	vector<ColumnBinding> GetColumnBindings() override;
	void ResolveTypes() override;

private:
	LogicalDelete(ClientContext &context, const unique_ptr<CreateInfo> &table_info);
};
} // namespace duckdb
