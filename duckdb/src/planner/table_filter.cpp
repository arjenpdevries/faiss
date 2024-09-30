#include "duckdb/planner/table_filter.hpp"

#include "duckdb/execution/operator/scan/physical_table_scan.hpp"
#include "duckdb/planner/filter/conjunction_filter.hpp"
#include "duckdb/planner/filter/constant_filter.hpp"
#include "duckdb/planner/filter/null_filter.hpp"

namespace duckdb {

void TableFilterSet::PushFilter(idx_t column_index, unique_ptr<TableFilter> filter) {
	auto entry = filters.find(column_index);
	if (entry == filters.end()) {
		// no filter yet: push the filter directly
		filters[column_index] = std::move(filter);
	} else {
		// there is already a filter: AND it together
		if (entry->second->filter_type == TableFilterType::CONJUNCTION_AND) {
		} else {
		}
	}
}

bool DynamicTableFilterSet::HasFilters() const {
	lock_guard<mutex> l(lock);
	return !filters.empty();
}

unique_ptr<TableFilterSet>
DynamicTableFilterSet::GetFinalTableFilters(const PhysicalTableScan &scan,
                                            optional_ptr<TableFilterSet> existing_filters) const {
	D_ASSERT(HasFilters());
	auto result = make_uniq<TableFilterSet>();
	if (existing_filters) {
		for (auto &entry : existing_filters->filters) {
			result->filters[entry.first] = entry.second->Copy();
		}
	}
	for (auto &entry : filters) {
		for (auto &filter : entry.second->filters) {
			result->filters[filter.first] = filter.second->Copy();
		}
	}
	if (result->filters.empty()) {
		return nullptr;
	}
	return result;
}

} // namespace duckdb
