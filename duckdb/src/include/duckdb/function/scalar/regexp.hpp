//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/function/scalar/regexp.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/function/built_in_functions.hpp"
#include "duckdb/function/function_set.hpp"
#include "re2/re2.h"
#include "re2/stringpiece.h"

namespace duckdb {

namespace regexp_util {

bool TryParseConstantPattern(ClientContext &context, Expression &expr, string &constant_string);
void ParseRegexOptions(const string &options, duckdb_re2::RE2::Options &result, bool *global_replace = nullptr);
void ParseRegexOptions(ClientContext &context, Expression &expr, RE2::Options &target, bool *global_replace = nullptr);

inline duckdb_re2::StringPiece CreateStringPiece(const string_t &input) {
	return duckdb_re2::StringPiece(input.GetData(), input.GetSize());
}

inline string_t Extract(const string_t &input, Vector &result, const RE2 &re, const duckdb_re2::StringPiece &rewrite) {
	string extracted;
	RE2::Extract(input.GetString(), re, rewrite, &extracted);
	return StringVector::AddString(result, extracted.c_str(), extracted.size());
}

} // namespace regexp_util

struct RegexpExtractAll {
	static void Execute(DataChunk &args, ExpressionState &state, Vector &result);
	static unique_ptr<FunctionData> Bind(ClientContext &context, ScalarFunction &bound_function,
	                                     vector<unique_ptr<Expression>> &arguments);
	static unique_ptr<FunctionLocalState> InitLocalState(ExpressionState &state, const BoundFunctionExpression &expr,
	                                                     FunctionData *bind_data);
};

struct RegexStringPieceArgs {
	RegexStringPieceArgs() : size(0), capacity(0), group_buffer(nullptr) {
	}
	void Init(idx_t size) {
		this->size = size;
		// Allocate for one extra, for the all-encompassing match group
		this->capacity = size + 1;
		group_buffer = AllocateArray<duckdb_re2::StringPiece>(capacity);
	}
	void SetSize(idx_t size) {
		this->size = size;
		if (size + 1 > capacity) {
			Clear();
			Init(size);
		}
	}

	RegexStringPieceArgs &operator=(RegexStringPieceArgs &&other) noexcept {
		this->size = other.size;
		this->capacity = other.capacity;
		this->group_buffer = other.group_buffer;
		other.size = 0;
		other.capacity = 0;
		other.group_buffer = nullptr;
		return *this;
	}

	~RegexStringPieceArgs() {
		Clear();
	}

private:
	void Clear() {
		DeleteArray<duckdb_re2::StringPiece>(group_buffer, capacity);
		group_buffer = nullptr;

		size = 0;
		capacity = 0;
	}

public:
	idx_t size;
	//! The currently allocated capacity for the groups
	idx_t capacity;
	//! Used by ExtractAll to pre-allocate the storage for the groups
	duckdb_re2::StringPiece *group_buffer;
};

unique_ptr<FunctionLocalState> RegexInitLocalState(ExpressionState &state, const BoundFunctionExpression &expr,
                                                   FunctionData *bind_data);
unique_ptr<FunctionData> RegexpMatchesBind(ClientContext &context, ScalarFunction &bound_function,
                                           vector<unique_ptr<Expression>> &arguments);

} // namespace duckdb
