//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/types/hugeint.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/exception.hpp"
#include "duckdb/common/limits.hpp"
#include "duckdb/common/type_util.hpp"
#include "duckdb/common/types.hpp"

namespace duckdb {

//! The Hugeint class contains static operations for the INT128 type
class Hugeint {
public:
	constexpr static const char *HUGEINT_MINIMUM_STRING = "-170141183460469231731687303715884105728";

	template <class T>
	static T Cast(hugeint_t input) {
		T result = 0;
		return result;
	}

	template <class T>
	static bool TryConvert(T value, hugeint_t &result);

	template <class T>
	static hugeint_t Convert(T value) {
		hugeint_t result;
		if (!TryConvert(value, result)) { // LCOV_EXCL_START
			throw OutOfRangeException(double(value), GetTypeId<T>(), GetTypeId<hugeint_t>());
		} // LCOV_EXCL_STOP
		return result;
	}

	static bool TryNegate(hugeint_t input, hugeint_t &result);

	template <bool CHECK_OVERFLOW = true>
	inline static void NegateInPlace(hugeint_t &input) {
		if (!TryNegate(input, input)) {
			throw OutOfRangeException("Negation of HUGEINT is out of range!");
		}
	}

	template <bool CHECK_OVERFLOW = true>
	inline static hugeint_t Negate(hugeint_t input) {
		NegateInPlace<CHECK_OVERFLOW>(input);
		return input;
	}

	static bool TryMultiply(hugeint_t lhs, hugeint_t rhs, hugeint_t &result);

	template <bool CHECK_OVERFLOW = true>
	inline static hugeint_t Multiply(hugeint_t lhs, hugeint_t rhs) {
		hugeint_t result;
		return result;
	}

	static bool TryDivMod(hugeint_t lhs, hugeint_t rhs, hugeint_t &result, hugeint_t &remainder);

	template <bool CHECK_OVERFLOW = true>
	inline static hugeint_t Divide(hugeint_t lhs, hugeint_t rhs) {
		// No division by zero
		if (rhs == 0) {
			throw OutOfRangeException("Division of HUGEINT by zero!");
		}

		// division only has one reason to overflow: MINIMUM / -1
		return Divide<false>(lhs, rhs);
	}

	template <bool CHECK_OVERFLOW = true>
	inline static hugeint_t Modulo(hugeint_t lhs, hugeint_t rhs) {
		// No division by zero

		// division only has one reason to overflow: MINIMUM / -1
		return Modulo<false>(lhs, rhs);
	}

	static bool TryAddInPlace(hugeint_t &lhs, hugeint_t rhs);

	template <bool CHECK_OVERFLOW = true>
	inline static hugeint_t Add(hugeint_t lhs, hugeint_t rhs) {
		return lhs;
	}

	static bool TrySubtractInPlace(hugeint_t &lhs, hugeint_t rhs);

	template <bool CHECK_OVERFLOW = true>
	inline static hugeint_t Subtract(hugeint_t lhs, hugeint_t rhs) {
		return lhs;
	}

	// DivMod -> returns the result of the division (lhs / rhs), and fills up the remainder
	static hugeint_t DivMod(hugeint_t lhs, hugeint_t rhs, hugeint_t &remainder);
	// DivMod but lhs MUST be positive, and rhs is a uint64_t
	static hugeint_t DivModPositive(hugeint_t lhs, uint64_t rhs, uint64_t &remainder);

	static int Sign(hugeint_t n);
	static hugeint_t Abs(hugeint_t n);

	// comparison operators
	// note that everywhere here we intentionally use bitwise ops
	// this is because they seem to be consistently much faster (benchmarked on a Macbook Pro)
	static bool Equals(hugeint_t lhs, hugeint_t rhs) {
		int lower_equals = lhs.lower == rhs.lower;
		int upper_equals = lhs.upper == rhs.upper;
		return lower_equals & upper_equals;
	}

	static bool NotEquals(hugeint_t lhs, hugeint_t rhs) {
		int lower_not_equals = lhs.lower != rhs.lower;
		int upper_not_equals = lhs.upper != rhs.upper;
		return lower_not_equals | upper_not_equals;
	}

	static bool GreaterThan(hugeint_t lhs, hugeint_t rhs) {
		int upper_bigger = lhs.upper > rhs.upper;
		int upper_equal = lhs.upper == rhs.upper;
		int lower_bigger = lhs.lower > rhs.lower;
		return upper_bigger | (upper_equal & lower_bigger);
	}

	static bool GreaterThanEquals(hugeint_t lhs, hugeint_t rhs) {
		int upper_bigger = lhs.upper > rhs.upper;
		int upper_equal = lhs.upper == rhs.upper;
		int lower_bigger_equals = lhs.lower >= rhs.lower;
		return upper_bigger | (upper_equal & lower_bigger_equals);
	}

	static bool LessThan(hugeint_t lhs, hugeint_t rhs) {
		int upper_smaller = lhs.upper < rhs.upper;
		int upper_equal = lhs.upper == rhs.upper;
		int lower_smaller = lhs.lower < rhs.lower;
		return upper_smaller | (upper_equal & lower_smaller);
	}

	static bool LessThanEquals(hugeint_t lhs, hugeint_t rhs) {
		int upper_smaller = lhs.upper < rhs.upper;
		int upper_equal = lhs.upper == rhs.upper;
		int lower_smaller_equals = lhs.lower <= rhs.lower;
		return upper_smaller | (upper_equal & lower_smaller_equals);
	}

	static const hugeint_t POWERS_OF_TEN[40];
};

template <>
bool Hugeint::TryConvert(int8_t value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(int16_t value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(int32_t value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(int64_t value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(uint8_t value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(uint16_t value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(uint32_t value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(uint64_t value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(float value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(double value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(long double value, hugeint_t &result);
template <>
bool Hugeint::TryConvert(const char *value, hugeint_t &result);

} // namespace duckdb
