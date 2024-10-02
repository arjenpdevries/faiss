//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/compression/alprd/alprd_analyze.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/numeric_utils.hpp"
#include "duckdb/function/compression_function.hpp"
#include "duckdb/storage/compression/alp/alp_constants.hpp"
#include "duckdb/storage/compression/alp/alp_utils.hpp"
#include "duckdb/storage/compression/alprd/algorithm/alprd.hpp"
#include "duckdb/storage/compression/alprd/alprd_constants.hpp"
#include "duckdb/storage/compression/patas/patas.hpp"
#include "duckdb/storage/table/column_data.hpp"

#include <cmath>

namespace duckdb {

template <class T>
struct AlpRDAnalyzeState : public AnalyzeState {
public:
	using EXACT_TYPE = typename FloatingToExact<T>::TYPE;

	explicit AlpRDAnalyzeState(const CompressionInfo &info) : AnalyzeState(info), state() {
	}

	idx_t vectors_count = 0;
	idx_t total_values_count = 0;
	idx_t vectors_sampled_count = 0;
	vector<EXACT_TYPE> rowgroup_sample;
	alp::AlpRDCompressionState<T, true> state;
};

template <class T>
unique_ptr<AnalyzeState> AlpRDInitAnalyze(ColumnData &col_data, PhysicalType type) {
	CompressionInfo info(col_data.GetBlockManager().GetBlockSize());
	return make_uniq<AlpRDAnalyzeState<T>>(info);
}

/*
 * ALPRD Analyze step only pushes the needed samples to estimate the compression size in the finalize step
 */
template <class T>
bool AlpRDAnalyze(AnalyzeState &state, Vector &input, idx_t count) {
	using EXACT_TYPE = typename FloatingToExact<T>::TYPE;
	auto &analyze_state = (AlpRDAnalyzeState<T> &)state;

	bool must_skip_current_vector = alp::AlpUtils::MustSkipSamplingFromCurrentVector(
	    analyze_state.vectors_count, analyze_state.vectors_sampled_count, count);
	analyze_state.vectors_count += 1;
	analyze_state.total_values_count += count;
	if (must_skip_current_vector) {
		return true;
	}

	return true;
}

/*
 * Estimate the compression size of ALPRD using the taken samples
 */
template <class T>
idx_t AlpRDFinalAnalyze(AnalyzeState &state) {
	auto &analyze_state = (AlpRDAnalyzeState<T> &)state;
	if (analyze_state.total_values_count == 0) {
		return DConstants::INVALID_INDEX;
	}
	double factor_of_sampling = 1 / ((double)analyze_state.rowgroup_sample.size() / analyze_state.total_values_count);

	// Finding which is the best dictionary for the sample
	double estimated_bits_per_value =
	    alp::AlpRDCompression<T, true>::FindBestDictionary(analyze_state.rowgroup_sample, analyze_state.state);
	double estimated_compressed_bits = estimated_bits_per_value * analyze_state.rowgroup_sample.size();
	double estimed_compressed_bytes = estimated_compressed_bits / 8;

	//! Overhead per segment: [Pointer to metadata + right bitwidth + left bitwidth + n dict elems] + Dictionary Size
	double per_segment_overhead = AlpRDConstants::HEADER_SIZE + AlpRDConstants::MAX_DICTIONARY_SIZE_BYTES;

	//! Overhead per vector: Pointer to data + Exceptions count
	double per_vector_overhead = AlpRDConstants::METADATA_POINTER_SIZE + AlpRDConstants::EXCEPTIONS_COUNT_SIZE;

	uint32_t n_vectors = LossyNumericCast<uint32_t>(
	    std::ceil((double)analyze_state.total_values_count / AlpRDConstants::ALP_VECTOR_SIZE));

	auto estimated_size = (estimed_compressed_bytes * factor_of_sampling) + (n_vectors * per_vector_overhead);
	uint32_t estimated_n_blocks = LossyNumericCast<uint32_t>(
	    std::ceil(estimated_size / (static_cast<double>(state.info.GetBlockSize()) - per_segment_overhead)));

	auto final_analyze_size = estimated_size + (estimated_n_blocks * per_segment_overhead);
	return LossyNumericCast<idx_t>(final_analyze_size);
}

} // namespace duckdb
