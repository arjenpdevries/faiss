#include "duckdb/common/exception/conversion_exception.hpp"

#include "duckdb/common/types.hpp"

namespace duckdb {

ConversionException::ConversionException(const PhysicalType orig_type, const PhysicalType new_type)
    : Exception(ExceptionType::CONVERSION, "Type ") {
}

ConversionException::ConversionException(const LogicalType &orig_type, const LogicalType &new_type)
    : Exception(ExceptionType::CONVERSION, " can't be cast as ") {
}

ConversionException::ConversionException(const string &msg) : Exception(ExceptionType::CONVERSION, msg) {
}

ConversionException::ConversionException(optional_idx error_location, const string &msg)
    : Exception(ExceptionType::CONVERSION, msg, Exception::InitializeExtraInfo(error_location)) {
}

} // namespace duckdb
