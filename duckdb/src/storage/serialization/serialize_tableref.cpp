//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_serialization.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/parser/tableref/list.hpp"

namespace duckdb {

void TableRef::Serialize(Serializer &serializer) const {
}

unique_ptr<TableRef> TableRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

unique_ptr<TableRef> BaseTableRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

unique_ptr<TableRef> ColumnDataRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

unique_ptr<TableRef> EmptyTableRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

unique_ptr<TableRef> ExpressionListRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

unique_ptr<TableRef> JoinRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

unique_ptr<TableRef> PivotRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

unique_ptr<TableRef> ShowRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

unique_ptr<TableRef> SubqueryRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

unique_ptr<TableRef> TableFunctionRef::Deserialize(Deserializer &deserializer) {
	return nullptr;
}

} // namespace duckdb