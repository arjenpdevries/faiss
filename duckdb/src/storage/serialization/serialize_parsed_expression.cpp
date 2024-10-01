//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_serialization.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/parser/expression/list.hpp"

namespace duckdb {

void ParsedExpression::Serialize(Serializer &serializer) const {
	serializer.WriteProperty<ExpressionClass>(100, "class", expression_class);
	serializer.WriteProperty<ExpressionType>(101, "type", type);
	serializer.WritePropertyWithDefault<string>(102, "alias", alias);
	serializer.WritePropertyWithDefault<optional_idx>(103, "query_location", query_location, optional_idx());
}

unique_ptr<ParsedExpression> ParsedExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void BetweenExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(200, "input", input);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(201, "lower", lower);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(202, "upper", upper);
}

unique_ptr<ParsedExpression> BetweenExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void CaseExpression::Serialize(Serializer &serializer) const {
}

unique_ptr<ParsedExpression> CaseExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void CastExpression::Serialize(Serializer &serializer) const {
}

unique_ptr<ParsedExpression> CastExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void CollateExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(200, "child", child);
	serializer.WritePropertyWithDefault<string>(201, "collation", collation);
}

unique_ptr<ParsedExpression> CollateExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void ColumnRefExpression::Serialize(Serializer &serializer) const {
}

unique_ptr<ParsedExpression> ColumnRefExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void ComparisonExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(200, "left", left);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(201, "right", right);
}

unique_ptr<ParsedExpression> ComparisonExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void ConjunctionExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(200, "children", children);
}

unique_ptr<ParsedExpression> ConjunctionExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void ConstantExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WriteProperty<Value>(200, "value", value);
}

unique_ptr<ParsedExpression> ConstantExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void DefaultExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
}

unique_ptr<ParsedExpression> DefaultExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void FunctionExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "function_name", function_name);
	serializer.WritePropertyWithDefault<string>(201, "schema", schema);
	serializer.WritePropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(202, "children", children);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(203, "filter", filter);
	serializer.WritePropertyWithDefault<unique_ptr<OrderModifier>>(204, "order_bys", order_bys);
	serializer.WritePropertyWithDefault<bool>(205, "distinct", distinct);
	serializer.WritePropertyWithDefault<bool>(206, "is_operator", is_operator);
	serializer.WritePropertyWithDefault<bool>(207, "export_state", export_state);
	serializer.WritePropertyWithDefault<string>(208, "catalog", catalog);
}

unique_ptr<ParsedExpression> FunctionExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void LambdaExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(200, "lhs", lhs);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(201, "expr", expr);
}

unique_ptr<ParsedExpression> LambdaExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void LambdaRefExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<idx_t>(200, "lambda_idx", lambda_idx);
	serializer.WritePropertyWithDefault<string>(201, "column_name", column_name);
}

unique_ptr<ParsedExpression> LambdaRefExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void OperatorExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(200, "children", children);
}

unique_ptr<ParsedExpression> OperatorExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void ParameterExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "identifier", identifier);
}

unique_ptr<ParsedExpression> ParameterExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void PositionalReferenceExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<idx_t>(200, "index", index);
}

unique_ptr<ParsedExpression> PositionalReferenceExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void StarExpression::Serialize(Serializer &serializer) const {
}

unique_ptr<ParsedExpression> StarExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void SubqueryExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WriteProperty<SubqueryType>(200, "subquery_type", subquery_type);
	serializer.WritePropertyWithDefault<unique_ptr<SelectStatement>>(201, "subquery", subquery);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(202, "child", child);
	serializer.WriteProperty<ExpressionType>(203, "comparison_type", comparison_type);
}

unique_ptr<ParsedExpression> SubqueryExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

void WindowExpression::Serialize(Serializer &serializer) const {
	ParsedExpression::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "function_name", function_name);
	serializer.WritePropertyWithDefault<string>(201, "schema", schema);
	serializer.WritePropertyWithDefault<string>(202, "catalog", catalog);
	serializer.WritePropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(203, "children", children);
	serializer.WritePropertyWithDefault<vector<unique_ptr<ParsedExpression>>>(204, "partitions", partitions);
	serializer.WritePropertyWithDefault<vector<OrderByNode>>(205, "orders", orders);
	serializer.WriteProperty<WindowBoundary>(206, "start", start);
	serializer.WriteProperty<WindowBoundary>(207, "end", end);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(208, "start_expr", start_expr);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(209, "end_expr", end_expr);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(210, "offset_expr", offset_expr);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(211, "default_expr", default_expr);
	serializer.WritePropertyWithDefault<bool>(212, "ignore_nulls", ignore_nulls);
	serializer.WritePropertyWithDefault<unique_ptr<ParsedExpression>>(213, "filter_expr", filter_expr);
	serializer.WritePropertyWithDefault<WindowExcludeMode>(214, "exclude_clause", exclude_clause,
	                                                       WindowExcludeMode::NO_OTHER);
	serializer.WritePropertyWithDefault<bool>(215, "distinct", distinct);
}

unique_ptr<ParsedExpression> WindowExpression::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

} // namespace duckdb