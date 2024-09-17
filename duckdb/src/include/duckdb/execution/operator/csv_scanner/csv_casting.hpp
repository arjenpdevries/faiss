//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/csv_scanner/csv_casting.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/operator/cast_operators.hpp"
#include "duckdb/common/operator/decimal_cast_operators.hpp"
#include "duckdb/common/vector_operations/unary_executor.hpp"
#include "duckdb/execution/operator/csv_scanner/csv_reader_options.hpp"

namespace duckdb {
class CSVCast {
	template <class OP, class T>
	static bool TemplatedTryCastFloatingVector(const CSVReaderOptions &options, Vector &input_vector,
	                                           Vector &result_vector, idx_t count, CastParameters &parameters,
	                                           idx_t &line_error) {
		D_ASSERT(input_vector.GetType().id() == LogicalTypeId::VARCHAR);
		bool all_converted = true;
		idx_t row = 0;
		UnaryExecutor::Execute<string_t, T>(input_vector, result_vector, count, [&](string_t input) {
			T result;
			if (!OP::Operation(input, result, parameters)) {
				line_error = row;
				all_converted = false;
			} else {
				row++;
			}
			return result;
		});
		return all_converted;
	}

	template <class OP, class T>
	static bool TemplatedTryCastDecimalVector(const CSVReaderOptions &options, Vector &input_vector,
	                                          Vector &result_vector, idx_t count, CastParameters &parameters,
	                                          uint8_t width, uint8_t scale, idx_t &line_error) {
		D_ASSERT(input_vector.GetType().id() == LogicalTypeId::VARCHAR);
		bool all_converted = true;
		auto &validity_mask = FlatVector::Validity(result_vector);
		idx_t cur_line = 0;
		UnaryExecutor::Execute<string_t, T>(input_vector, result_vector, count, [&](string_t input) {
			T result;
			if (!OP::Operation(input, result, parameters, width, scale)) {
				if (all_converted) {
					line_error = cur_line;
				}
				validity_mask.SetInvalid(cur_line);
				all_converted = false;
				cur_line++;
			} else {
				cur_line++;
			}
			return result;
		});
		return all_converted;
	}

public:
	static bool TryCastFloatingVectorCommaSeparated(const CSVReaderOptions &options, Vector &input_vector,
	                                                Vector &result_vector, idx_t count, CastParameters &parameters,
	                                                const LogicalType &result_type, idx_t &line_error) {
		switch (result_type.InternalType()) {
		case PhysicalType::DOUBLE:
			return TemplatedTryCastFloatingVector<TryCastErrorMessageCommaSeparated, double>(
			    options, input_vector, result_vector, count, parameters, line_error);
		case PhysicalType::FLOAT:
			return TemplatedTryCastFloatingVector<TryCastErrorMessageCommaSeparated, float>(
			    options, input_vector, result_vector, count, parameters, line_error);
		default:
			throw InternalException("Unimplemented physical type for floating");
		}
	}

	static bool TryCastDecimalVectorCommaSeparated(const CSVReaderOptions &options, Vector &input_vector,
	                                               Vector &result_vector, idx_t count, CastParameters &parameters,
	                                               const LogicalType &result_type, idx_t &line_error) {
		auto width = DecimalType::GetWidth(result_type);
		auto scale = DecimalType::GetScale(result_type);
		switch (result_type.InternalType()) {
		case PhysicalType::INT16:
			return TemplatedTryCastDecimalVector<TryCastToDecimalCommaSeparated, int16_t>(
			    options, input_vector, result_vector, count, parameters, width, scale, line_error);
		case PhysicalType::INT32:
			return TemplatedTryCastDecimalVector<TryCastToDecimalCommaSeparated, int32_t>(
			    options, input_vector, result_vector, count, parameters, width, scale, line_error);
		case PhysicalType::INT64:
			return TemplatedTryCastDecimalVector<TryCastToDecimalCommaSeparated, int64_t>(
			    options, input_vector, result_vector, count, parameters, width, scale, line_error);
		case PhysicalType::INT128:
			return TemplatedTryCastDecimalVector<TryCastToDecimalCommaSeparated, hugeint_t>(
			    options, input_vector, result_vector, count, parameters, width, scale, line_error);
		default:
			throw InternalException("Unimplemented physical type for decimal");
		}
	}
};
} // namespace duckdb
