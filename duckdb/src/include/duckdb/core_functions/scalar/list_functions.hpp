//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/core_functions/scalar/list_functions.hpp
//
//
//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_functions.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/function/function_set.hpp"

namespace duckdb {

struct ListDistinctFun {
	static constexpr const char *Name = "list_distinct";
	static constexpr const char *Parameters = "list";
	static constexpr const char *Description =
	    "Removes all duplicates and NULLs from a list. Does not preserve the original order";
	static constexpr const char *Example = "list_distinct([1, 1, NULL, -3, 1, 5])";

	static ScalarFunction GetFunction();
};

struct ArrayDistinctFun {
	using ALIAS = ListDistinctFun;

	static constexpr const char *Name = "array_distinct";
};

struct ListUniqueFun {
	static constexpr const char *Name = "list_unique";
	static constexpr const char *Parameters = "list";
	static constexpr const char *Description = "Counts the unique elements of a list";
	static constexpr const char *Example = "list_unique([1, 1, NULL, -3, 1, 5])";

	static ScalarFunction GetFunction();
};

struct ArrayUniqueFun {
	using ALIAS = ListUniqueFun;

	static constexpr const char *Name = "array_unique";
};

struct ListValueFun {
	static constexpr const char *Name = "list_value";
	static constexpr const char *Parameters = "any,...";
	static constexpr const char *Description = "Create a LIST containing the argument values";
	static constexpr const char *Example = "list_value(4, 5, 6)";

	static ScalarFunction GetFunction();
};

struct ListPackFun {
	using ALIAS = ListValueFun;

	static constexpr const char *Name = "list_pack";
};

struct ListSliceFun {
	static constexpr const char *Name = "list_slice";
	static constexpr const char *Parameters = "list,begin,end[,step]";
	static constexpr const char *Description =
	    "Extract a sublist using slice conventions. Negative values are accepted";
	static constexpr const char *Example = "list_slice(l, 2, 4)";

	static ScalarFunctionSet GetFunctions();
};

struct ArraySliceFun {
	using ALIAS = ListSliceFun;

	static constexpr const char *Name = "array_slice";
};

struct ListSortFun {
	static constexpr const char *Name = "list_sort";
	static constexpr const char *Parameters = "list";
	static constexpr const char *Description = "Sorts the elements of the list";
	static constexpr const char *Example = "list_sort([3, 6, 1, 2])";

	static ScalarFunctionSet GetFunctions();
};

struct ArraySortFun {
	using ALIAS = ListSortFun;

	static constexpr const char *Name = "array_sort";
};

struct ListReverseSortFun {
	static constexpr const char *Name = "list_reverse_sort";
	static constexpr const char *Parameters = "list";
	static constexpr const char *Description = "Sorts the elements of the list in reverse order";
	static constexpr const char *Example = "list_reverse_sort([3, 6, 1, 2])";

	static ScalarFunctionSet GetFunctions();
};

struct ArrayReverseSortFun {
	using ALIAS = ListReverseSortFun;

	static constexpr const char *Name = "array_reverse_sort";
};

struct ListFilterFun {
	static constexpr const char *Name = "list_filter";
	static constexpr const char *Parameters = "list,lambda";
	static constexpr const char *Description =
	    "Constructs a list from those elements of the input list for which the lambda function returns true";
	static constexpr const char *Example = "list_filter([3, 4, 5], x -> x > 4)";

	static ScalarFunction GetFunction();
};

struct ArrayFilterFun {
	using ALIAS = ListFilterFun;

	static constexpr const char *Name = "array_filter";
};

struct FilterFun {
	using ALIAS = ListFilterFun;

	static constexpr const char *Name = "filter";
};

struct ListRangeFun {
	static constexpr const char *Name = "range";
	static constexpr const char *Parameters = "start,stop,step";
	static constexpr const char *Description =
	    "Create a list of values between start and stop - the stop parameter is exclusive";
	static constexpr const char *Example = "range(2, 5, 3)";

	static ScalarFunctionSet GetFunctions();
};

struct ListCosineDistanceFun {
	static constexpr const char *Name = "list_cosine_distance";
	static constexpr const char *Parameters = "list1,list2";
	static constexpr const char *Description = "Compute the cosine distance between two lists";
	static constexpr const char *Example = "list_cosine_distance([1, 2, 3], [1, 2, 3])";

	static ScalarFunctionSet GetFunctions();
};

struct ListCosineDistanceFunAlias {
	using ALIAS = ListCosineDistanceFun;

	static constexpr const char *Name = "<=>";
};

struct ListCosineSimilarityFun {
	static constexpr const char *Name = "list_cosine_similarity";
	static constexpr const char *Parameters = "list1,list2";
	static constexpr const char *Description = "Compute the cosine similarity between two lists";
	static constexpr const char *Example = "list_cosine_similarity([1, 2, 3], [1, 2, 3])";

	static ScalarFunctionSet GetFunctions();
};

struct ListDistanceFun {
	static constexpr const char *Name = "list_distance";
	static constexpr const char *Parameters = "list1,list2";
	static constexpr const char *Description = "Compute the distance between two lists";
	static constexpr const char *Example = "list_distance([1, 2, 3], [1, 2, 3])";

	static ScalarFunctionSet GetFunctions();
};

struct ListDistanceFunAlias {
	using ALIAS = ListDistanceFun;

	static constexpr const char *Name = "<->";
};

struct ListInnerProductFun {
	static constexpr const char *Name = "list_inner_product";
	static constexpr const char *Parameters = "list1,list2";
	static constexpr const char *Description = "Compute the inner product between two lists";
	static constexpr const char *Example = "list_inner_product([1, 2, 3], [1, 2, 3])";

	static ScalarFunctionSet GetFunctions();
};

struct ListDotProductFun {
	using ALIAS = ListInnerProductFun;

	static constexpr const char *Name = "list_dot_product";
};

struct ListNegativeInnerProductFun {
	static constexpr const char *Name = "list_negative_inner_product";
	static constexpr const char *Parameters = "list1,list2";
	static constexpr const char *Description = "Compute the negative inner product between two lists";
	static constexpr const char *Example = "list_negative_inner_product([1, 2, 3], [1, 2, 3])";

	static ScalarFunctionSet GetFunctions();
};

struct ListNegativeDotProductFun {
	using ALIAS = ListNegativeInnerProductFun;

	static constexpr const char *Name = "list_negative_dot_product";
};

struct ListNegativeInnerProductFunAlias {
	using ALIAS = ListNegativeInnerProductFun;

	static constexpr const char *Name = "<#>";
};

struct UnpivotListFun {
	static constexpr const char *Name = "unpivot_list";
	static constexpr const char *Parameters = "any,...";
	static constexpr const char *Description =
	    "Identical to list_value, but generated as part of unpivot for better error messages";
	static constexpr const char *Example = "unpivot_list(4, 5, 6)";

	static ScalarFunction GetFunction();
};

struct ListHasAnyFun {
	static constexpr const char *Name = "list_has_any";
	static constexpr const char *Parameters = "l1, l2";
	static constexpr const char *Description =
	    "Returns true if the lists have any element in common. NULLs are ignored.";
	static constexpr const char *Example = "list_has_any([1, 2, 3], [2, 3, 4])";

	static ScalarFunction GetFunction();
};

struct ArrayHasAnyFun {
	using ALIAS = ListHasAnyFun;

	static constexpr const char *Name = "array_has_any";
};

struct ListHasAnyFunAlias {
	using ALIAS = ListHasAnyFun;

	static constexpr const char *Name = "&&";
};

struct ListHasAllFun {
	static constexpr const char *Name = "list_has_all";
	static constexpr const char *Parameters = "l1, l2";
	static constexpr const char *Description = "Returns true if all elements of l2 are in l1. NULLs are ignored.";
	static constexpr const char *Example = "list_has_all([1, 2, 3], [2, 3])";

	static ScalarFunction GetFunction();
};

struct ArrayHasAllFun {
	using ALIAS = ListHasAllFun;

	static constexpr const char *Name = "array_has_all";
};

struct ListHasAllFunAlias {
	using ALIAS = ListHasAllFun;

	static constexpr const char *Name = "@>";
};

struct ListHasAllFunAlias2 {
	using ALIAS = ListHasAllFun;

	static constexpr const char *Name = "<@";
};

} // namespace duckdb
