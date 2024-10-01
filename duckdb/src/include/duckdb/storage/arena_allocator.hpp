//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/arena_allocator.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/allocator.hpp"
#include "duckdb/common/common.hpp"

namespace duckdb {

struct ArenaChunk {
	ArenaChunk(Allocator &allocator, idx_t size);

	AllocatedData data;
	idx_t current_position;
	idx_t maximum_size;
	unsafe_unique_ptr<ArenaChunk> next;
	ArenaChunk *prev;
};

class ArenaAllocator {
	static constexpr const idx_t ARENA_ALLOCATOR_INITIAL_CAPACITY = 2048;
	static constexpr const idx_t ARENA_ALLOCATOR_MAX_CAPACITY = 1ULL << 24ULL; // 16MB

public:
	DUCKDB_API data_ptr_t Reallocate(data_ptr_t pointer, idx_t old_size, idx_t size);

	DUCKDB_API data_ptr_t AllocateAligned(idx_t size);
	DUCKDB_API data_ptr_t ReallocateAligned(data_ptr_t pointer, idx_t old_size, idx_t size);

	//! Increment the internal cursor (if required) so the next allocation is guaranteed to be aligned to 8 bytes
	DUCKDB_API void AlignNext();

	DUCKDB_API ArenaChunk *GetHead();
	DUCKDB_API ArenaChunk *GetTail();

	DUCKDB_API bool IsEmpty() const;
	//! Get the total *used* size (not cached)
	DUCKDB_API idx_t SizeInBytes() const;
	//! Get the currently allocated size in bytes (cached, read from "allocated_size")
	DUCKDB_API idx_t AllocationSize() const;

	//! Returns an "Allocator" wrapper for this arena allocator
	Allocator &GetAllocator() {
		return arena_allocator;
	}

private:
	idx_t initial_capacity;
	unsafe_unique_ptr<ArenaChunk> head;
	ArenaChunk *tail;
	//! An allocator wrapper using this arena allocator
	Allocator arena_allocator;
	//! The total allocated size
	idx_t allocated_size = 0;
};

} // namespace duckdb
