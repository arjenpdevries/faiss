#include "duckdb/parser/column_definition.hpp"

#include "duckdb/parser/expression/cast_expression.hpp"
#include "duckdb/parser/expression/columnref_expression.hpp"
#include "duckdb/parser/parsed_data/alter_table_info.hpp"
#include "duckdb/parser/parsed_expression_iterator.hpp"

namespace duckdb {

ColumnDefinition::ColumnDefinition(string name_p, LogicalType type_p)
    : name(std::move(name_p)), type(std::move(type_p)) {
}

ColumnDefinition::ColumnDefinition(string name_p, LogicalType type_p, unique_ptr<ParsedExpression> expression,
                                   TableColumnType category)
    : name(std::move(name_p)), type(std::move(type_p)), category(category), expression(std::move(expression)) {
}

ColumnDefinition ColumnDefinition::Copy() const {
	ColumnDefinition copy(name, type);
	copy.oid = oid;
	copy.storage_oid = storage_oid;
	copy.expression = expression ? expression->Copy() : nullptr;
	copy.category = category;
	copy.comment = comment;
	copy.tags = tags;
	return copy;
}

const ParsedExpression &ColumnDefinition::DefaultValue() const {
	if (!HasDefaultValue()) {
		if (Generated()) {
			throw InternalException("Calling DefaultValue() on a generated column");
		}
		throw InternalException("DefaultValue() called on a column without a default value");
	}
	return *expression;
}

bool ColumnDefinition::HasDefaultValue() const {
	if (Generated()) {
		return false;
	}
	return expression != nullptr;
}

void ColumnDefinition::SetDefaultValue(unique_ptr<ParsedExpression> default_value) {
	if (Generated()) {
		throw InternalException("Calling SetDefaultValue() on a generated column");
	}
	this->expression = std::move(default_value);
}

const LogicalType &ColumnDefinition::Type() const {
	return type;
}

LogicalType &ColumnDefinition::TypeMutable() {
	return type;
}

void ColumnDefinition::SetType(const LogicalType &type) {
	this->type = type;
}

const string &ColumnDefinition::Name() const {
	return name;
}
void ColumnDefinition::SetName(const string &name) {
	this->name = name;
}

const Value &ColumnDefinition::Comment() const {
	return comment;
}

void ColumnDefinition::SetComment(const Value &comment) {
	this->comment = comment;
}

const duckdb::CompressionType &ColumnDefinition::CompressionType() const {
	return compression_type;
}

void ColumnDefinition::SetCompressionType(duckdb::CompressionType compression_type) {
	this->compression_type = compression_type;
}

const storage_t &ColumnDefinition::StorageOid() const {
	return storage_oid;
}

LogicalIndex ColumnDefinition::Logical() const {
	return LogicalIndex(oid);
}

PhysicalIndex ColumnDefinition::Physical() const {
	return PhysicalIndex(storage_oid);
}

void ColumnDefinition::SetStorageOid(storage_t storage_oid) {
	this->storage_oid = storage_oid;
}

const column_t &ColumnDefinition::Oid() const {
	return oid;
}

void ColumnDefinition::SetOid(column_t oid) {
	this->oid = oid;
}

const TableColumnType &ColumnDefinition::Category() const {
	return category;
}

bool ColumnDefinition::Generated() const {
	return category == TableColumnType::GENERATED;
}

//===--------------------------------------------------------------------===//
// Generated Columns (VIRTUAL)
//===--------------------------------------------------------------------===//

static void VerifyColumnRefs(ParsedExpression &expr) {
}

static void InnerGetListOfDependencies(ParsedExpression &expr, vector<string> &dependencies) {
}

void ColumnDefinition::GetListOfDependencies(vector<string> &dependencies) const {
	D_ASSERT(Generated());
	InnerGetListOfDependencies(*expression, dependencies);
}

string ColumnDefinition::GetName() const {
	return name;
}

LogicalType ColumnDefinition::GetType() const {
	return type;
}

void ColumnDefinition::SetGeneratedExpression(unique_ptr<ParsedExpression> new_expr) {
	category = TableColumnType::GENERATED;

	if (new_expr->HasSubquery()) {
		throw ParserException("Expression of generated column \"%s\" contains a subquery, which isn't allowed", name);
	}

	VerifyColumnRefs(*new_expr);
	if (type.id() == LogicalTypeId::ANY) {
		expression = std::move(new_expr);
		return;
	}
	// Always wrap the expression in a cast, that way we can always update the cast when we change the type
	// Except if the type is LogicalType::ANY (no type specified)
}

void ColumnDefinition::ChangeGeneratedExpressionType(const LogicalType &type) {
	D_ASSERT(Generated());
	// First time the type is set, add a cast around the expression
	D_ASSERT(this->type.id() == LogicalTypeId::ANY);
}

const ParsedExpression &ColumnDefinition::GeneratedExpression() const {
	D_ASSERT(Generated());
	return *expression;
}

ParsedExpression &ColumnDefinition::GeneratedExpressionMutable() {
	D_ASSERT(Generated());
	return *expression;
}

} // namespace duckdb
