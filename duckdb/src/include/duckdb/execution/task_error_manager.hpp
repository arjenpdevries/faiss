//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/task_error_manager.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/error_data.hpp"
#include "duckdb/common/mutex.hpp"
#include "duckdb/common/vector.hpp"

namespace duckdb {

class TaskErrorManager {
public:
	void PushError(ErrorData error) {
	}

	bool HasError() {
		return false;
	}

	void ThrowException() {
	}

	void Reset() {
	}

private:
	mutex error_lock;
	//! Exceptions that occurred during the execution of the current query
	vector<ErrorData> exceptions;
};
} // namespace duckdb
