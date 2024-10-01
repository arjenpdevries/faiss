//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/catalog/catalog_entry/table_catalog_entry.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/catalog/catalog_entry/column_dependency_manager.hpp"
#include "duckdb/catalog/catalog_entry/table_column_type.hpp"
#include "duckdb/catalog/standard_entry.hpp"
#include "duckdb/common/case_insensitive_map.hpp"
#include "duckdb/common/unordered_map.hpp"
#include "duckdb/parser/column_list.hpp"
#include "duckdb/parser/constraint.hpp"
#include "duckdb/planner/bound_constraint.hpp"
#include "duckdb/planner/expression.hpp"

namespace duckdb {

class DataTable;
struct CreateTableInfo;
struct BoundCreateTableInfo;

struct RenameColumnInfo;
struct AddColumnInfo;
struct RemoveColumnInfo;
struct SetDefaultInfo;
struct ChangeColumnTypeInfo;
struct AlterForeignKeyInfo;
struct SetNotNullInfo;
struct DropNotNullInfo;
struct SetColumnCommentInfo;

class TableFunction;
struct FunctionData;

class Binder;
class TableColumnInfo;
struct ColumnSegmentInfo;
class TableStorageInfo;

class LogicalGet;
class LogicalProjection;
class LogicalUpdate;

//! A table catalog entry
class TableCatalogEntry : public StandardEntry {
public:
	static constexpr const CatalogType Type = CatalogType::TABLE_ENTRY;
	static constexpr const char *Name = "table";

public:
	//! Create a TableCatalogEntry and initialize storage for it
	DUCKDB_API TableCatalogEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateTableInfo &info);

public:
	DUCKDB_API unique_ptr<CreateInfo> GetInfo() const;

	//! Returns whether or not a column with the given name exists
	DUCKDB_API bool ColumnExists(const string &name) const;
	//! Returns a reference to the column of the specified name. Throws an
	//! exception if the column does not exist.
	//! Returns a list of types of the table, excluding generated columns
	virtual DataTable &GetStorage();

	//! Returns a list of the constraints of the table
	DUCKDB_API string ToSQL() const;

	//! Returns the column index of the specified column name.
	//! If the column does not exist:
	//! If if_column_exists is true, returns DConstants::INVALID_INDEX
	//! If if_column_exists is false, throws an exception
	DUCKDB_API LogicalIndex GetColumnIndex(string &name, bool if_exists = false) const;

	//! Returns the scan function that can be used to scan the given table
	virtual TableFunction GetScanFunction(ClientContext &context, unique_ptr<FunctionData> &bind_data) = 0;

	virtual bool IsDuckTable() const {
		return false;
	}

	DUCKDB_API static string ColumnsToSQL(const ColumnList &columns, const vector<unique_ptr<Constraint>> &constraints);

	//! Returns a list of segment information for this table, if exists
	virtual vector<ColumnSegmentInfo> GetColumnSegmentInfo();

	//! Returns the storage info of this table
	virtual TableStorageInfo GetStorageInfo(ClientContext &context) = 0;

	virtual void BindUpdateConstraints(Binder &binder, LogicalGet &get, LogicalProjection &proj, LogicalUpdate &update,
	                                   ClientContext &context);

protected:
	//! A list of columns that are part of this table
	ColumnList columns;
	//! A list of constraints that are part of this table
	vector<unique_ptr<Constraint>> constraints;
};
} // namespace duckdb