//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/main/capi/cast/generic_cast.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/types/date.hpp"
#include "duckdb/common/types/time.hpp"
#include "duckdb/common/types/timestamp.hpp"
#include "duckdb/main/capi/capi_internal.hpp"
#include "duckdb/main/capi/cast/from_decimal.hpp"
#include "duckdb/main/capi/cast/utils.hpp"

namespace duckdb {

template <class RESULT_TYPE>
RESULT_TYPE GetInternalCValue(duckdb_result *result, idx_t col, idx_t row) {
	if (!CanFetchValue(result, col, row)) {
		return FetchDefaultValue::Operation<RESULT_TYPE>();
	}
	return FetchDefaultValue::Operation<RESULT_TYPE>();
}

} // namespace duckdb
