//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/planner/expression.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/types.hpp"
#include "duckdb/parser/base_expression.hpp"

namespace duckdb {
class BaseStatistics;
class ClientContext;

//!  The Expression class represents a bound Expression with a return type
class Expression : public BaseExpression {
public:
	Expression(ExpressionType type, ExpressionClass expression_class, LogicalType return_type);
	~Expression() override;

	//! The return type of the expression
	LogicalType return_type;

public:
	bool IsAggregate() const override;
	bool IsWindow() const override;
	bool HasSubquery() const override;
	bool IsScalar() const override;
	bool HasParameter() const override;
	virtual bool IsVolatile() const;
	virtual bool IsConsistent() const;
	virtual bool PropagatesNullValues() const;
	virtual bool IsFoldable() const;

	hash_t Hash() const override;

	static bool ListEquals(const vector<unique_ptr<Expression>> &left, const vector<unique_ptr<Expression>> &right);
	//! Create a copy of this expression
	virtual unique_ptr<Expression> Copy() const = 0;

protected:
	//! Copy base Expression properties from another expression to this one,
	//! used in Copy method
	void CopyProperties(const Expression &other) {
		type = other.type;
		expression_class = other.expression_class;
		alias = other.alias;
		return_type = other.return_type;
		query_location = other.query_location;
	}
};

} // namespace duckdb
