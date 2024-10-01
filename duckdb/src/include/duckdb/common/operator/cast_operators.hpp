//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/operator/cast_operators.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/assert.hpp"
#include "duckdb/common/constants.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/exception/conversion_exception.hpp"
#include "duckdb/common/hugeint.hpp"
#include "duckdb/common/limits.hpp"
#include "duckdb/common/operator/convert_to_string.hpp"
#include "duckdb/common/typedefs.hpp"
#include "duckdb/common/types.hpp"
#include "duckdb/common/types/bit.hpp"
#include "duckdb/common/types/null_value.hpp"
#include "duckdb/common/types/string_type.hpp"
#include "duckdb/common/types/vector.hpp"
#include "duckdb/function/cast/default_casts.hpp"

namespace duckdb {
struct CastParameters;
struct ValidityMask;
class Vector;

struct TryCast {
	template <class SRC, class DST>
	static inline bool Operation(SRC input, DST &result, bool strict = false) {
		throw NotImplementedException("Unimplemented type for cast (%s -> %s)", GetTypeId<SRC>(), GetTypeId<DST>());
	}
};

struct TryCastErrorMessage {
	template <class SRC, class DST>
	static inline bool Operation(SRC input, DST &result, CastParameters &parameters) {
		throw NotImplementedException(parameters.query_location, "Unimplemented type for cast (%s -> %s)",
		                              GetTypeId<SRC>(), GetTypeId<DST>());
	}
};

struct TryCastErrorMessageCommaSeparated {
	template <class SRC, class DST>
	static inline bool Operation(SRC input, DST &result, CastParameters &parameters) {
		throw NotImplementedException(parameters.query_location, "Unimplemented type for cast (%s -> %s)",
		                              GetTypeId<SRC>(), GetTypeId<DST>());
	}
};

template <class SRC, class DST>
static string CastExceptionText(SRC input) {
	if (std::is_same<SRC, string_t>()) {
		return "Could not convert string '" + ConvertToString::Operation<SRC>(input) + "' to " +
		       TypeIdToString(GetTypeId<DST>());
	}
	if (TypeIsNumber<SRC>() && TypeIsNumber<DST>()) {
		return "Type " + TypeIdToString(GetTypeId<SRC>()) + " with value " + ConvertToString::Operation<SRC>(input) +
		       " can't be cast because the value is out of range for the destination type " +
		       TypeIdToString(GetTypeId<DST>());
	}
	return "Type " + TypeIdToString(GetTypeId<SRC>()) + " with value " + ConvertToString::Operation<SRC>(input) +
	       " can't be cast to the destination type " + TypeIdToString(GetTypeId<DST>());
}

struct Cast {};

struct HandleCastError {
	static void AssignError(const string &error_message, CastParameters &parameters);
	static void AssignError(const string &error_message, string *error_message_ptr,
	                        optional_idx error_location = optional_idx()) {
		if (!error_message_ptr) {
			throw ConversionException(error_location, error_message);
		}
		if (error_message_ptr->empty()) {
			*error_message_ptr = error_message;
		}
	}
};

//===--------------------------------------------------------------------===//
// string -> Non-Standard Timestamps
//===--------------------------------------------------------------------===//
struct TryCastToTimestampNS {
	template <class SRC, class DST>
	static inline bool Operation(SRC input, DST &result, bool strict = false) {
		throw InternalException("Unsupported type for try cast to timestamp (ns)");
	}
};

struct TryCastToTimestampMS {
	template <class SRC, class DST>
	static inline bool Operation(SRC input, DST &result, bool strict = false) {
		throw InternalException("Unsupported type for try cast to timestamp (ms)");
	}
};

struct TryCastToTimestampSec {
	template <class SRC, class DST>
	static inline bool Operation(SRC input, DST &result, bool strict = false) {
		throw InternalException("Unsupported type for try cast to timestamp (s)");
	}
};

template <>
DUCKDB_API bool TryCastToTimestampNS::Operation(string_t input, timestamp_ns_t &result, bool strict);
template <>
DUCKDB_API bool TryCastToTimestampMS::Operation(string_t input, timestamp_t &result, bool strict);
template <>
DUCKDB_API bool TryCastToTimestampSec::Operation(string_t input, timestamp_t &result, bool strict);

template <>
DUCKDB_API bool TryCastToTimestampNS::Operation(date_t input, timestamp_ns_t &result, bool strict);
template <>
DUCKDB_API bool TryCastToTimestampMS::Operation(date_t input, timestamp_t &result, bool strict);
template <>
DUCKDB_API bool TryCastToTimestampSec::Operation(date_t input, timestamp_t &result, bool strict);

//===--------------------------------------------------------------------===//
// Non-Standard Timestamps -> string/timestamp types
//===--------------------------------------------------------------------===//

struct CastFromTimestampNS {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		throw duckdb::NotImplementedException("Cast to string could not be performed!");
	}
};

struct CastFromTimestampMS {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		throw duckdb::NotImplementedException("Cast to string could not be performed!");
	}
};

struct CastFromTimestampSec {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		throw duckdb::NotImplementedException("Cast to string could not be performed!");
	}
};

struct CastTimestampUsToMs {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to timestamp could not be performed!");
	}
};

struct CastTimestampUsToNs {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to timestamp could not be performed!");
	}
};

struct CastTimestampUsToSec {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to timestamp could not be performed!");
	}
};

struct CastTimestampMsToDate {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to DATE could not be performed!");
	}
};

struct CastTimestampMsToTime {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to TIME could not be performed!");
	}
};

struct CastTimestampMsToUs {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to timestamp could not be performed!");
	}
};

struct CastTimestampMsToNs {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to TIMESTAMP_NS could not be performed!");
	}
};

struct CastTimestampNsToDate {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to DATE could not be performed!");
	}
};
struct CastTimestampNsToTime {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to TIME could not be performed!");
	}
};
struct CastTimestampNsToUs {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to timestamp could not be performed!");
	}
};

struct CastTimestampSecToDate {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to DATE could not be performed!");
	}
};
struct CastTimestampSecToTime {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to TIME could not be performed!");
	}
};
struct CastTimestampSecToMs {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to TIMESTAMP_MS could not be performed!");
	}
};

struct CastTimestampSecToUs {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to timestamp could not be performed!");
	}
};

struct CastTimestampSecToNs {
	template <class SRC, class DST>
	static inline DST Operation(SRC input) {
		throw duckdb::NotImplementedException("Cast to TIMESTAMP_NS could not be performed!");
	}
};

template <>
duckdb::timestamp_t CastTimestampUsToSec::Operation(duckdb::timestamp_t input);
template <>
duckdb::timestamp_t CastTimestampUsToMs::Operation(duckdb::timestamp_t input);
template <>
duckdb::timestamp_t CastTimestampUsToNs::Operation(duckdb::timestamp_t input);
template <>
duckdb::date_t CastTimestampMsToDate::Operation(duckdb::timestamp_t input);
template <>
duckdb::dtime_t CastTimestampMsToTime::Operation(duckdb::timestamp_t input);
template <>
duckdb::timestamp_t CastTimestampMsToUs::Operation(duckdb::timestamp_t input);
template <>
duckdb::timestamp_t CastTimestampMsToNs::Operation(duckdb::timestamp_t input);
template <>
duckdb::date_t CastTimestampNsToDate::Operation(duckdb::timestamp_t input);
template <>
duckdb::dtime_t CastTimestampNsToTime::Operation(duckdb::timestamp_t input);
template <>
duckdb::timestamp_t CastTimestampNsToUs::Operation(duckdb::timestamp_t input);
template <>
duckdb::date_t CastTimestampSecToDate::Operation(duckdb::timestamp_t input);
template <>
duckdb::dtime_t CastTimestampSecToTime::Operation(duckdb::timestamp_t input);
template <>
duckdb::timestamp_t CastTimestampSecToMs::Operation(duckdb::timestamp_t input);
template <>
duckdb::timestamp_t CastTimestampSecToUs::Operation(duckdb::timestamp_t input);
template <>
duckdb::timestamp_t CastTimestampSecToNs::Operation(duckdb::timestamp_t input);

template <>
duckdb::string_t CastFromTimestampNS::Operation(duckdb::timestamp_ns_t input, Vector &result);
template <>
duckdb::string_t CastFromTimestampMS::Operation(duckdb::timestamp_t input, Vector &result);
template <>
duckdb::string_t CastFromTimestampSec::Operation(duckdb::timestamp_t input, Vector &result);

//===--------------------------------------------------------------------===//
// Blobs
//===--------------------------------------------------------------------===//
struct CastFromBlob {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		throw duckdb::NotImplementedException("Cast from blob could not be performed!");
	}
};
template <>
duckdb::string_t CastFromBlob::Operation(duckdb::string_t input, Vector &vector);

struct CastFromBlobToBit {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		throw NotImplementedException("Cast from blob could not be performed!");
	}
};
template <>
string_t CastFromBlobToBit::Operation(string_t input, Vector &result);

struct TryCastToBlob {
	template <class SRC, class DST>
	static inline bool Operation(SRC input, DST &result, Vector &result_vector, CastParameters &parameters) {
		throw InternalException("Unsupported type for try cast to blob");
	}
};
template <>
bool TryCastToBlob::Operation(string_t input, string_t &result, Vector &result_vector, CastParameters &parameters);

//===--------------------------------------------------------------------===//
// Bits
//===--------------------------------------------------------------------===//
struct CastFromBitToString {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		throw duckdb::NotImplementedException("Cast from bit could not be performed!");
	}
};
template <>
duckdb::string_t CastFromBitToString::Operation(duckdb::string_t input, Vector &vector);

struct CastFromBitToNumeric {
	template <class SRC = string_t, class DST>
	static inline bool Operation(SRC input, DST &result, CastParameters &parameters) {
		D_ASSERT(input.GetSize() > 1);

		// TODO: Allow conversion if the significant bytes of the bitstring can be cast to the target type
		// Currently only allows bitstring -> numeric if the full bitstring fits inside the numeric type
		if (input.GetSize() - 1 > sizeof(DST)) {
			throw ConversionException(parameters.query_location, "Bitstring doesn't fit inside of %s",
			                          GetTypeId<DST>());
		}
		Bit::BitToNumeric(input, result);
		return (true);
	}
};
template <>
bool CastFromBitToNumeric::Operation(string_t input, bool &result, CastParameters &parameters);
template <>
bool CastFromBitToNumeric::Operation(string_t input, hugeint_t &result, CastParameters &parameters);
template <>
bool CastFromBitToNumeric::Operation(string_t input, uhugeint_t &result, CastParameters &parameters);

struct CastFromBitToBlob {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		D_ASSERT(input.GetSize() > 1);
		return StringVector::AddStringOrBlob(result, Bit::BitToBlob(input));
	}
};

struct TryCastToBit {
	template <class SRC, class DST>
	static inline bool Operation(SRC input, DST &result, Vector &result_vector, CastParameters &parameters) {
		throw InternalException("Unsupported type for try cast to bit");
	}
};

template <>
bool TryCastToBit::Operation(string_t input, string_t &result, Vector &result_vector, CastParameters &parameters);

//===--------------------------------------------------------------------===//
// UUID
//===--------------------------------------------------------------------===//
struct CastFromUUID {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		throw duckdb::NotImplementedException("Cast from uuid could not be performed!");
	}
};
template <>
duckdb::string_t CastFromUUID::Operation(duckdb::hugeint_t input, Vector &vector);

struct TryCastToUUID {
	template <class SRC, class DST>
	static inline bool Operation(SRC input, DST &result, Vector &result_vector, CastParameters &parameters) {
		throw InternalException("Unsupported type for try cast to uuid");
	}
};

template <>
DUCKDB_API bool TryCastToUUID::Operation(string_t input, hugeint_t &result, Vector &result_vector,
                                         CastParameters &parameters);

//===--------------------------------------------------------------------===//
// Pointers
//===--------------------------------------------------------------------===//
struct CastFromPointer {
	template <class SRC>
	static inline string_t Operation(SRC input, Vector &result) {
		throw duckdb::NotImplementedException("Cast from pointer could not be performed!");
	}
};
template <>
duckdb::string_t CastFromPointer::Operation(uintptr_t input, Vector &vector);

} // namespace duckdb
