#include "duckdb/main/relation.hpp"

#include "duckdb/common/printer.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/parser/expression/columnref_expression.hpp"
#include "duckdb/parser/expression/conjunction_expression.hpp"
#include "duckdb/parser/parser.hpp"
#include "duckdb/parser/statement/explain_statement.hpp"
#include "duckdb/parser/statement/select_statement.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"
#include "duckdb/planner/binder.hpp"

namespace duckdb {

static vector<unique_ptr<ParsedExpression>> StringListToExpressionList(ClientContext &context,
                                                                       const vector<string> &expressions) {
	if (expressions.empty()) {
		throw ParserException("Zero expressions provided");
	}
	vector<unique_ptr<ParsedExpression>> result_list;
	for (auto &expr : expressions) {
		auto expression_list = Parser::ParseExpressionList(expr, context.GetParserOptions());
		if (expression_list.size() != 1) {
			throw ParserException("Expected a single expression in the expression list");
		}
		result_list.push_back(std::move(expression_list[0]));
	}
	return result_list;
}

shared_ptr<Relation> Relation::Project(const vector<string> &expressions, const vector<string> &aliases) {
	return nullptr;
}

shared_ptr<Relation> Relation::Filter(const string &expression) {
	auto expression_list = Parser::ParseExpressionList(expression, context.GetContext()->GetParserOptions());
	if (expression_list.size() != 1) {
		throw ParserException("Expected a single expression as filter condition");
	}
	return Filter(std::move(expression_list[0]));
}

shared_ptr<Relation> Relation::Filter(unique_ptr<ParsedExpression> expression) {
	return nullptr;
}

shared_ptr<Relation> Relation::Filter(const vector<string> &expressions) {
	return nullptr;
}

shared_ptr<Relation> Relation::Limit(int64_t limit, int64_t offset) {
	return nullptr;
}

shared_ptr<Relation> Relation::Order(const string &expression) {
	auto order_list = Parser::ParseOrderList(expression, context.GetContext()->GetParserOptions());
	return Order(std::move(order_list));
}

shared_ptr<Relation> Relation::Order(vector<OrderByNode> order_list) {
	return nullptr;
}

shared_ptr<Relation> Relation::Order(const vector<string> &expressions) {
	if (expressions.empty()) {
		throw ParserException("Zero ORDER BY expressions provided");
	}
	vector<OrderByNode> order_list;
	for (auto &expression : expressions) {
		auto inner_list = Parser::ParseOrderList(expression, context.GetContext()->GetParserOptions());
		if (inner_list.size() != 1) {
			throw ParserException("Expected a single ORDER BY expression in the expression list");
		}
		order_list.push_back(std::move(inner_list[0]));
	}
	return Order(std::move(order_list));
}

shared_ptr<Relation> Relation::Join(const shared_ptr<Relation> &other, const string &condition, JoinType type,
                                    JoinRefType ref_type) {
	auto expression_list = Parser::ParseExpressionList(condition, context.GetContext()->GetParserOptions());
	D_ASSERT(!expression_list.empty());
	return Join(other, std::move(expression_list), type, ref_type);
}

shared_ptr<Relation> Relation::Join(const shared_ptr<Relation> &other,
                                    vector<unique_ptr<ParsedExpression>> expression_list, JoinType type,
                                    JoinRefType ref_type) {
	return nullptr;
}

shared_ptr<Relation> Relation::CrossProduct(const shared_ptr<Relation> &other, JoinRefType join_ref_type) {
	return nullptr;
}

shared_ptr<Relation> Relation::Union(const shared_ptr<Relation> &other) {
	return nullptr;
}

shared_ptr<Relation> Relation::Except(const shared_ptr<Relation> &other) {
	return nullptr;
}

shared_ptr<Relation> Relation::Intersect(const shared_ptr<Relation> &other) {
	return nullptr;
}

shared_ptr<Relation> Relation::Distinct() {
	return nullptr;
}

shared_ptr<Relation> Relation::Alias(const string &alias) {
	return nullptr;
}

shared_ptr<Relation> Relation::Aggregate(const string &aggregate_list) {
	return nullptr;
}

shared_ptr<Relation> Relation::Aggregate(vector<unique_ptr<ParsedExpression>> expressions) {
	return nullptr;
}

shared_ptr<Relation> Relation::Aggregate(const string &aggregate_list, const string &group_list) {
	return nullptr;
}

shared_ptr<Relation> Relation::Aggregate(const vector<string> &aggregates) {
	return nullptr;
}

shared_ptr<Relation> Relation::Aggregate(const vector<string> &aggregates, const vector<string> &groups) {
	auto aggregate_list = StringUtil::Join(aggregates, ", ");
	auto group_list = StringUtil::Join(groups, ", ");
	return this->Aggregate(aggregate_list, group_list);
}

shared_ptr<Relation> Relation::Aggregate(vector<unique_ptr<ParsedExpression>> expressions, const string &group_list) {
	return nullptr;
}

string Relation::GetAlias() {
	return "relation";
}

unique_ptr<TableRef> Relation::GetTableRef() {
	auto select = make_uniq<SelectStatement>();
	select->node = GetQueryNode();
	return make_uniq<SubqueryRef>(std::move(select), GetAlias());
}

unique_ptr<QueryResult> Relation::Execute() {
	return context.GetContext()->Execute(shared_from_this());
}

unique_ptr<QueryResult> Relation::ExecuteOrThrow() {
	auto res = Execute();
	D_ASSERT(res);
	if (res->HasError()) {
		res->ThrowError();
	}
	return res;
}

BoundStatement Relation::Bind(Binder &binder) {
	throw InvalidInputException("UPDATE can only be used on base tables!");
}

shared_ptr<Relation> Relation::InsertRel(const string &schema_name, const string &table_name) {
	return nullptr;
}

void Relation::Insert(const string &table_name) {
	Insert(INVALID_SCHEMA, table_name);
}

void Relation::Insert(const string &schema_name, const string &table_name) {
	auto insert = InsertRel(schema_name, table_name);
	auto res = insert->Execute();
	if (res->HasError()) {
		const string prepended_message = "Failed to insert into table '" + table_name + "': ";
		res->ThrowError(prepended_message);
	}
}

void Relation::Insert(const vector<vector<Value>> &values) {
}

shared_ptr<Relation> Relation::CreateRel(const string &schema_name, const string &table_name, bool temporary) {
	return nullptr;
}

void Relation::Create(const string &table_name, bool temporary) {
	Create(INVALID_SCHEMA, table_name, temporary);
}

void Relation::Create(const string &schema_name, const string &table_name, bool temporary) {
	auto create = CreateRel(schema_name, table_name, temporary);
	auto res = create->Execute();
	if (res->HasError()) {
		const string prepended_message = "Failed to create table '" + table_name + "': ";
		res->ThrowError(prepended_message);
	}
}

shared_ptr<Relation> Relation::WriteCSVRel(const string &csv_file, case_insensitive_map_t<vector<Value>> options) {
	return nullptr;
}

void Relation::WriteCSV(const string &csv_file, case_insensitive_map_t<vector<Value>> options) {
	auto write_csv = WriteCSVRel(csv_file, std::move(options));
	auto res = write_csv->Execute();
	if (res->HasError()) {
		const string prepended_message = "Failed to write '" + csv_file + "': ";
		res->ThrowError(prepended_message);
	}
}

shared_ptr<Relation> Relation::WriteParquetRel(const string &parquet_file,
                                               case_insensitive_map_t<vector<Value>> options) {
	return nullptr;
}

void Relation::WriteParquet(const string &parquet_file, case_insensitive_map_t<vector<Value>> options) {
	auto write_parquet = WriteParquetRel(parquet_file, std::move(options));
	auto res = write_parquet->Execute();
	if (res->HasError()) {
		const string prepended_message = "Failed to write '" + parquet_file + "': ";
		res->ThrowError(prepended_message);
	}
}

shared_ptr<Relation> Relation::CreateView(const string &name, bool replace, bool temporary) {
	return CreateView(INVALID_SCHEMA, name, replace, temporary);
}

shared_ptr<Relation> Relation::CreateView(const string &schema_name, const string &name, bool replace, bool temporary) {
	return nullptr;
}

unique_ptr<QueryResult> Relation::Query(const string &sql) {
	return context.GetContext()->Query(sql, false);
}

unique_ptr<QueryResult> Relation::Query(const string &name, const string &sql) {
	CreateView(name);
	return Query(sql);
}

unique_ptr<QueryResult> Relation::Explain(ExplainType type, ExplainFormat format) {
	return nullptr;
}

void Relation::Update(const string &update, const string &condition) {
	throw InvalidInputException("UPDATE can only be used on base tables!");
}

void Relation::Delete(const string &condition) {
	throw InvalidInputException("DELETE can only be used on base tables!");
}

shared_ptr<Relation> Relation::TableFunction(const std::string &fname, const vector<Value> &values,
                                             const named_parameter_map_t &named_parameters) {
	return nullptr;
}

shared_ptr<Relation> Relation::TableFunction(const std::string &fname, const vector<Value> &values) {
	return nullptr;
}

string Relation::ToString() {
	string str;
	str += "---------------------\n";
	str += "--- Relation Tree ---\n";
	str += "---------------------\n";
	str += ToString(0);
	str += "\n\n";
	str += "---------------------\n";
	str += "-- Result Columns  --\n";
	str += "---------------------\n";
	auto &cols = Columns();
	for (idx_t i = 0; i < cols.size(); i++) {
		str += "- " + cols[i].Name() + " (" + cols[i].Type().ToString() + ")\n";
	}
	return str;
}

// LCOV_EXCL_START
unique_ptr<QueryNode> Relation::GetQueryNode() {
	throw InternalException("Cannot create a query node from this node type");
}

void Relation::Head(idx_t limit) {
	auto limit_node = Limit(NumericCast<int64_t>(limit));
	limit_node->Execute()->Print();
}
// LCOV_EXCL_STOP

void Relation::Print() {
	Printer::Print(ToString());
}

string Relation::RenderWhitespace(idx_t depth) {
	return string(depth * 2, ' ');
}

void Relation::AddExternalDependency(shared_ptr<ExternalDependency> dependency) {
	external_dependencies.push_back(std::move(dependency));
}

vector<shared_ptr<ExternalDependency>> Relation::GetAllDependencies() {
	vector<shared_ptr<ExternalDependency>> all_dependencies;
	Relation *cur = this;
	while (cur) {
		for (auto &dep : cur->external_dependencies) {
			all_dependencies.push_back(dep);
		}
		cur = cur->ChildRelation();
	}
	return all_dependencies;
}

} // namespace duckdb
