#include "duckdb/storage/data_pointer.hpp"

#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/function/compression_function.hpp"
#include "duckdb/main/config.hpp"

namespace duckdb {

DataPointer::DataPointer() {
}

DataPointer &DataPointer::operator=(DataPointer &&other) noexcept {
	return *this;
}

unique_ptr<ColumnSegmentState> ColumnSegmentState::Deserialize(Deserializer &deserializer) {
	auto compression_type = deserializer.Get<CompressionType>();
	auto &db = deserializer.Get<DatabaseInstance &>();
	auto &type = deserializer.Get<const LogicalType &>();

	auto compression_function = DBConfig::GetConfig(db).GetCompressionFunction(compression_type, type.InternalType());
	if (!compression_function || !compression_function->deserialize_state) {
		throw SerializationException("Deserializing a ColumnSegmentState but could not find deserialize method");
	}
	return compression_function->deserialize_state(deserializer);
}

} // namespace duckdb
