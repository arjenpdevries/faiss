#include "duckdb/function/built_in_functions.hpp"

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/catalog/catalog_entry/scalar_function_catalog_entry.hpp"
#include "duckdb/parser/parsed_data/create_aggregate_function_info.hpp"
#include "duckdb/parser/parsed_data/create_collation_info.hpp"
#include "duckdb/parser/parsed_data/create_copy_function_info.hpp"
#include "duckdb/parser/parsed_data/create_pragma_function_info.hpp"
#include "duckdb/parser/parsed_data/create_scalar_function_info.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"

namespace duckdb {

BuiltinFunctions::BuiltinFunctions(CatalogTransaction transaction, Catalog &catalog) : transaction(transaction) {
}

BuiltinFunctions::~BuiltinFunctions() {
}

void BuiltinFunctions::AddCollation(string name, ScalarFunction function, bool combinable,
                                    bool not_required_for_equality) {
}

void BuiltinFunctions::AddFunction(AggregateFunctionSet set) {
}

void BuiltinFunctions::AddFunction(AggregateFunction function) {
}

void BuiltinFunctions::AddFunction(PragmaFunction function) {
}

void BuiltinFunctions::AddFunction(const string &name, PragmaFunctionSet functions) {
}

void BuiltinFunctions::AddFunction(ScalarFunction function) {
}

void BuiltinFunctions::AddFunction(const vector<string> &names, ScalarFunction function) { // NOLINT: false positive
}

void BuiltinFunctions::AddFunction(ScalarFunctionSet set) {
}

void BuiltinFunctions::AddFunction(TableFunction function) {
}

void BuiltinFunctions::AddFunction(TableFunctionSet set) {
}

void BuiltinFunctions::AddFunction(CopyFunction function) {
}

} // namespace duckdb
