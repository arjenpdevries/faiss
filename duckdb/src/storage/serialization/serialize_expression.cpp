//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_serialization.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/planner/expression/list.hpp"

namespace duckdb {

void Expression::Serialize(Serializer &serializer) const {
}

unique_ptr<Expression> Expression::Deserialize(Deserializer &deserializer) {
	auto expression_class = deserializer.ReadProperty<ExpressionClass>(100, "expression_class");
	auto type = deserializer.ReadProperty<ExpressionType>(101, "type");
	auto alias = deserializer.ReadPropertyWithDefault<string>(102, "alias");
	auto query_location =
	    deserializer.ReadPropertyWithExplicitDefault<optional_idx>(103, "query_location", optional_idx());
	deserializer.Set<ExpressionType>(type);
	unique_ptr<Expression> result;
	switch (expression_class) {
	case ExpressionClass::BOUND_AGGREGATE:
		result = BoundAggregateExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_BETWEEN:
		result = BoundBetweenExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_CASE:
		result = BoundCaseExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_COLUMN_REF:
		result = BoundColumnRefExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_COMPARISON:
		result = BoundComparisonExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_CONJUNCTION:
		result = BoundConjunctionExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_CONSTANT:
		result = BoundConstantExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_DEFAULT:
		result = BoundDefaultExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_FUNCTION:
		result = BoundFunctionExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_LAMBDA:
		result = BoundLambdaExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_LAMBDA_REF:
		result = BoundLambdaRefExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_OPERATOR:
		result = BoundOperatorExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_PARAMETER:
		result = BoundParameterExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_REF:
		result = BoundReferenceExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_UNNEST:
		result = BoundUnnestExpression::Deserialize(deserializer);
		break;
	case ExpressionClass::BOUND_WINDOW:
		result = BoundWindowExpression::Deserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of Expression!");
	}
	deserializer.Unset<ExpressionType>();
	result->alias = std::move(alias);
	result->query_location = query_location;
	return result;
}

void BoundBetweenExpression::Serialize(Serializer &serializer) const {
}

unique_ptr<Expression> BoundBetweenExpression::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<BoundBetweenExpression>(new BoundBetweenExpression());
	deserializer.ReadPropertyWithDefault<unique_ptr<Expression>>(200, "input", result->input);
	deserializer.ReadPropertyWithDefault<unique_ptr<Expression>>(201, "lower", result->lower);
	deserializer.ReadPropertyWithDefault<unique_ptr<Expression>>(202, "upper", result->upper);
	deserializer.ReadPropertyWithDefault<bool>(203, "lower_inclusive", result->lower_inclusive);
	deserializer.ReadPropertyWithDefault<bool>(204, "upper_inclusive", result->upper_inclusive);
	return std::move(result);
}

void BoundCaseExpression::Serialize(Serializer &serializer) const {
}

unique_ptr<Expression> BoundCaseExpression::Deserialize(Deserializer &deserializer) {
	auto return_type = deserializer.ReadProperty<LogicalType>(200, "return_type");
	auto result = duckdb::unique_ptr<BoundCaseExpression>(new BoundCaseExpression(std::move(return_type)));
	deserializer.ReadPropertyWithDefault<vector<BoundCaseCheck>>(201, "case_checks", result->case_checks);
	deserializer.ReadPropertyWithDefault<unique_ptr<Expression>>(202, "else_expr", result->else_expr);
	return std::move(result);
}

void BoundCastExpression::Serialize(Serializer &serializer) const {
}

unique_ptr<Expression> BoundCastExpression::Deserialize(Deserializer &deserializer) {
	auto child = deserializer.ReadPropertyWithDefault<unique_ptr<Expression>>(200, "child");
	auto return_type = deserializer.ReadProperty<LogicalType>(201, "return_type");
	return nullptr;
}

void BoundColumnRefExpression::Serialize(Serializer &serializer) const {
}

unique_ptr<Expression> BoundColumnRefExpression::Deserialize(Deserializer &deserializer) {
	auto return_type = deserializer.ReadProperty<LogicalType>(200, "return_type");
	auto binding = deserializer.ReadProperty<ColumnBinding>(201, "binding");
	auto depth = deserializer.ReadPropertyWithDefault<idx_t>(202, "depth");
	auto result = duckdb::unique_ptr<BoundColumnRefExpression>(
	    new BoundColumnRefExpression(std::move(return_type), binding, depth));
	return std::move(result);
}

void BoundComparisonExpression::Serialize(Serializer &serializer) const {
}

unique_ptr<Expression> BoundComparisonExpression::Deserialize(Deserializer &deserializer) {
	auto left = deserializer.ReadPropertyWithDefault<unique_ptr<Expression>>(200, "left");
	auto right = deserializer.ReadPropertyWithDefault<unique_ptr<Expression>>(201, "right");
	auto result = duckdb::unique_ptr<BoundComparisonExpression>(
	    new BoundComparisonExpression(deserializer.Get<ExpressionType>(), std::move(left), std::move(right)));
	return std::move(result);
}

void BoundConjunctionExpression::Serialize(Serializer &serializer) const {
	Expression::Serialize(serializer);
	serializer.WritePropertyWithDefault<vector<unique_ptr<Expression>>>(200, "children", children);
}

unique_ptr<Expression> BoundConjunctionExpression::Deserialize(Deserializer &deserializer) {
	auto result = duckdb::unique_ptr<BoundConjunctionExpression>(
	    new BoundConjunctionExpression(deserializer.Get<ExpressionType>()));
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<Expression>>>(200, "children", result->children);
	return std::move(result);
}

void BoundConstantExpression::Serialize(Serializer &serializer) const {
	Expression::Serialize(serializer);
	serializer.WriteProperty<Value>(200, "value", value);
}

unique_ptr<Expression> BoundConstantExpression::Deserialize(Deserializer &deserializer) {
	auto value = deserializer.ReadProperty<Value>(200, "value");
	auto result = duckdb::unique_ptr<BoundConstantExpression>(new BoundConstantExpression(value));
	return std::move(result);
}

void BoundDefaultExpression::Serialize(Serializer &serializer) const {
	Expression::Serialize(serializer);
	serializer.WriteProperty<LogicalType>(200, "return_type", return_type);
}

unique_ptr<Expression> BoundDefaultExpression::Deserialize(Deserializer &deserializer) {
	auto return_type = deserializer.ReadProperty<LogicalType>(200, "return_type");
	auto result = duckdb::unique_ptr<BoundDefaultExpression>(new BoundDefaultExpression(std::move(return_type)));
	return std::move(result);
}

void BoundLambdaExpression::Serialize(Serializer &serializer) const {
	Expression::Serialize(serializer);
	serializer.WriteProperty<LogicalType>(200, "return_type", return_type);
	serializer.WritePropertyWithDefault<unique_ptr<Expression>>(201, "lambda_expr", lambda_expr);
	serializer.WritePropertyWithDefault<vector<unique_ptr<Expression>>>(202, "captures", captures);
	serializer.WritePropertyWithDefault<idx_t>(203, "parameter_count", parameter_count);
}

unique_ptr<Expression> BoundLambdaExpression::Deserialize(Deserializer &deserializer) {
	auto return_type = deserializer.ReadProperty<LogicalType>(200, "return_type");
	auto lambda_expr = deserializer.ReadPropertyWithDefault<unique_ptr<Expression>>(201, "lambda_expr");
	auto captures = deserializer.ReadPropertyWithDefault<vector<unique_ptr<Expression>>>(202, "captures");
	auto parameter_count = deserializer.ReadPropertyWithDefault<idx_t>(203, "parameter_count");
	auto result = duckdb::unique_ptr<BoundLambdaExpression>(new BoundLambdaExpression(
	    deserializer.Get<ExpressionType>(), std::move(return_type), std::move(lambda_expr), parameter_count));
	result->captures = std::move(captures);
	return std::move(result);
}

void BoundLambdaRefExpression::Serialize(Serializer &serializer) const {
	Expression::Serialize(serializer);
	serializer.WriteProperty<LogicalType>(200, "return_type", return_type);
	serializer.WriteProperty<ColumnBinding>(201, "binding", binding);
	serializer.WritePropertyWithDefault<idx_t>(202, "lambda_index", lambda_idx);
	serializer.WritePropertyWithDefault<idx_t>(203, "depth", depth);
}

unique_ptr<Expression> BoundLambdaRefExpression::Deserialize(Deserializer &deserializer) {
	auto return_type = deserializer.ReadProperty<LogicalType>(200, "return_type");
	auto binding = deserializer.ReadProperty<ColumnBinding>(201, "binding");
	auto lambda_idx = deserializer.ReadPropertyWithDefault<idx_t>(202, "lambda_index");
	auto depth = deserializer.ReadPropertyWithDefault<idx_t>(203, "depth");
	auto result = duckdb::unique_ptr<BoundLambdaRefExpression>(
	    new BoundLambdaRefExpression(std::move(return_type), binding, lambda_idx, depth));
	return std::move(result);
}

void BoundOperatorExpression::Serialize(Serializer &serializer) const {
	Expression::Serialize(serializer);
	serializer.WriteProperty<LogicalType>(200, "return_type", return_type);
	serializer.WritePropertyWithDefault<vector<unique_ptr<Expression>>>(201, "children", children);
}

unique_ptr<Expression> BoundOperatorExpression::Deserialize(Deserializer &deserializer) {
	auto return_type = deserializer.ReadProperty<LogicalType>(200, "return_type");
	auto result = duckdb::unique_ptr<BoundOperatorExpression>(
	    new BoundOperatorExpression(deserializer.Get<ExpressionType>(), std::move(return_type)));
	deserializer.ReadPropertyWithDefault<vector<unique_ptr<Expression>>>(201, "children", result->children);
	return std::move(result);
}

void BoundParameterExpression::Serialize(Serializer &serializer) const {
	Expression::Serialize(serializer);
	serializer.WritePropertyWithDefault<string>(200, "identifier", identifier);
	serializer.WriteProperty<LogicalType>(201, "return_type", return_type);
	serializer.WritePropertyWithDefault<shared_ptr<BoundParameterData>>(202, "parameter_data", parameter_data);
}

unique_ptr<Expression> BoundParameterExpression::Deserialize(Deserializer &deserializer) {
	auto identifier = deserializer.ReadPropertyWithDefault<string>(200, "identifier");
	auto return_type = deserializer.ReadProperty<LogicalType>(201, "return_type");
	auto parameter_data = deserializer.ReadPropertyWithDefault<shared_ptr<BoundParameterData>>(202, "parameter_data");
	auto result = duckdb::unique_ptr<BoundParameterExpression>(
	    new BoundParameterExpression(deserializer.Get<bound_parameter_map_t &>(), std::move(identifier),
	                                 std::move(return_type), std::move(parameter_data)));
	return std::move(result);
}

void BoundReferenceExpression::Serialize(Serializer &serializer) const {
	Expression::Serialize(serializer);
	serializer.WriteProperty<LogicalType>(200, "return_type", return_type);
	serializer.WritePropertyWithDefault<idx_t>(201, "index", index);
}

unique_ptr<Expression> BoundReferenceExpression::Deserialize(Deserializer &deserializer) {
	auto return_type = deserializer.ReadProperty<LogicalType>(200, "return_type");
	auto index = deserializer.ReadPropertyWithDefault<idx_t>(201, "index");
	auto result =
	    duckdb::unique_ptr<BoundReferenceExpression>(new BoundReferenceExpression(std::move(return_type), index));
	return std::move(result);
}

void BoundUnnestExpression::Serialize(Serializer &serializer) const {
	Expression::Serialize(serializer);
	serializer.WriteProperty<LogicalType>(200, "return_type", return_type);
	serializer.WritePropertyWithDefault<unique_ptr<Expression>>(201, "child", child);
}

unique_ptr<Expression> BoundUnnestExpression::Deserialize(Deserializer &deserializer) {
	auto return_type = deserializer.ReadProperty<LogicalType>(200, "return_type");
	auto result = duckdb::unique_ptr<BoundUnnestExpression>(new BoundUnnestExpression(std::move(return_type)));
	deserializer.ReadPropertyWithDefault<unique_ptr<Expression>>(201, "child", result->child);
	return std::move(result);
}

} // namespace duckdb
