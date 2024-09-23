#include "duckdb/core_functions/function_list.hpp"

#include "duckdb/core_functions/aggregate/algebraic_functions.hpp"
#include "duckdb/core_functions/aggregate/distributive_functions.hpp"
#include "duckdb/core_functions/aggregate/holistic_functions.hpp"
#include "duckdb/core_functions/aggregate/nested_functions.hpp"
#include "duckdb/core_functions/aggregate/regression_functions.hpp"
#include "duckdb/core_functions/scalar/array_functions.hpp"
#include "duckdb/core_functions/scalar/bit_functions.hpp"
#include "duckdb/core_functions/scalar/blob_functions.hpp"
#include "duckdb/core_functions/scalar/date_functions.hpp"
#include "duckdb/core_functions/scalar/debug_functions.hpp"
#include "duckdb/core_functions/scalar/enum_functions.hpp"
#include "duckdb/core_functions/scalar/generic_functions.hpp"
#include "duckdb/core_functions/scalar/list_functions.hpp"
#include "duckdb/core_functions/scalar/map_functions.hpp"
#include "duckdb/core_functions/scalar/math_functions.hpp"
#include "duckdb/core_functions/scalar/operators_functions.hpp"
#include "duckdb/core_functions/scalar/random_functions.hpp"
#include "duckdb/core_functions/scalar/secret_functions.hpp"
#include "duckdb/core_functions/scalar/string_functions.hpp"
#include "duckdb/core_functions/scalar/struct_functions.hpp"
#include "duckdb/core_functions/scalar/union_functions.hpp"

namespace duckdb {

// Scalar Function
#define DUCKDB_SCALAR_FUNCTION_BASE(_PARAM, _NAME)                                                                     \
	{_NAME, _PARAM::Parameters, _PARAM::Description, _PARAM::Example, _PARAM::GetFunction, nullptr, nullptr, nullptr}
#define DUCKDB_SCALAR_FUNCTION(_PARAM)       DUCKDB_SCALAR_FUNCTION_BASE(_PARAM, _PARAM::Name)
#define DUCKDB_SCALAR_FUNCTION_ALIAS(_PARAM) DUCKDB_SCALAR_FUNCTION_BASE(_PARAM::ALIAS, _PARAM::Name)
// Scalar Function Set
#define DUCKDB_SCALAR_FUNCTION_SET_BASE(_PARAM, _NAME)                                                                 \
	{_NAME, _PARAM::Parameters, _PARAM::Description, _PARAM::Example, nullptr, _PARAM::GetFunctions, nullptr, nullptr}
#define DUCKDB_SCALAR_FUNCTION_SET(_PARAM)       DUCKDB_SCALAR_FUNCTION_SET_BASE(_PARAM, _PARAM::Name)
#define DUCKDB_SCALAR_FUNCTION_SET_ALIAS(_PARAM) DUCKDB_SCALAR_FUNCTION_SET_BASE(_PARAM::ALIAS, _PARAM::Name)
// Aggregate Function
#define DUCKDB_AGGREGATE_FUNCTION_BASE(_PARAM, _NAME)                                                                  \
	{_NAME, _PARAM::Parameters, _PARAM::Description, _PARAM::Example, nullptr, nullptr, _PARAM::GetFunction, nullptr}
#define DUCKDB_AGGREGATE_FUNCTION(_PARAM)       DUCKDB_AGGREGATE_FUNCTION_BASE(_PARAM, _PARAM::Name)
#define DUCKDB_AGGREGATE_FUNCTION_ALIAS(_PARAM) DUCKDB_AGGREGATE_FUNCTION_BASE(_PARAM::ALIAS, _PARAM::Name)
// Aggregate Function Set
#define DUCKDB_AGGREGATE_FUNCTION_SET_BASE(_PARAM, _NAME)                                                              \
	{_NAME, _PARAM::Parameters, _PARAM::Description, _PARAM::Example, nullptr, nullptr, nullptr, _PARAM::GetFunctions}
#define DUCKDB_AGGREGATE_FUNCTION_SET(_PARAM)       DUCKDB_AGGREGATE_FUNCTION_SET_BASE(_PARAM, _PARAM::Name)
#define DUCKDB_AGGREGATE_FUNCTION_SET_ALIAS(_PARAM) DUCKDB_AGGREGATE_FUNCTION_SET_BASE(_PARAM::ALIAS, _PARAM::Name)
#define FINAL_FUNCTION                              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}

// this list is generated by scripts/generate_functions.py
static const StaticFunctionDefinition internal_functions[] = {FINAL_FUNCTION};

const StaticFunctionDefinition *StaticFunctionDefinition::GetFunctionList() {
	return internal_functions;
}

} // namespace duckdb
