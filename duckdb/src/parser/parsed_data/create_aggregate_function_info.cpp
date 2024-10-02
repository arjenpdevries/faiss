#include "duckdb/parser/parsed_data/create_aggregate_function_info.hpp"

namespace duckdb {

unique_ptr<CreateInfo> CreateAggregateFunctionInfo::Copy() const {
	return nullptr;
}

} // namespace duckdb
