//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/aggregate_hashtable.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/row_operations/row_matcher.hpp"
#include "duckdb/common/types/row/partitioned_tuple_data.hpp"
#include "duckdb/execution/base_aggregate_hashtable.hpp"
#include "duckdb/execution/ht_entry.hpp"
#include "duckdb/storage/arena_allocator.hpp"
#include "duckdb/storage/buffer/buffer_handle.hpp"

namespace duckdb {

class BlockHandle;
class BufferHandle;

struct FlushMoveState;

//! GroupedAggregateHashTable is a linear probing HT that is used for computing
//! aggregates
/*!
    GroupedAggregateHashTable is a HT that is used for computing aggregates. It takes
   as input the set of groups and the types of the aggregates to compute and
   stores them in the HT. It uses linear probing for collision resolution.
*/

} // namespace duckdb
