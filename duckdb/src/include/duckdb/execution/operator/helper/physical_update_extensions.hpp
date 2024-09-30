//===----------------------------------------------------------------------===//
//                         DuckDB
//
// src/include/duckdb/execution/operator/helper/physical_update_extensions.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/main/extension_helper.hpp"
#include "duckdb/parser/parsed_data/update_extensions_info.hpp"

namespace duckdb {

struct UpdateExtensionsGlobalState : public GlobalSourceState {
	UpdateExtensionsGlobalState() : offset(0) {
	}

	vector<ExtensionUpdateResult> update_result_entries;
	idx_t offset;
};

} // namespace duckdb
