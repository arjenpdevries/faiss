#include "duckdb/common/numeric_utils.hpp"
#include "duckdb/common/sort/sort.hpp"
#include "duckdb/common/types/column/column_data_collection.hpp"
#include "duckdb/common/types/list_segment.hpp"
#include "duckdb/function/aggregate/distributive_functions.hpp"
#include "duckdb/function/aggregate_function.hpp"
#include "duckdb/function/function_binder.hpp"
#include "duckdb/parser/expression_map.hpp"
#include "duckdb/planner/expression/bound_aggregate_expression.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"
#include "duckdb/storage/buffer_manager.hpp"

namespace duckdb {

struct SortedAggregateBindData : public FunctionData {
	SortedAggregateBindData(ClientContext &context, BoundAggregateExpression &expr)
	    : function(expr.function), bind_info(std::move(expr.bind_info)),
	      threshold(ClientConfig::GetConfig(context).ordered_aggregate_threshold),
	      external(ClientConfig::GetConfig(context).force_external) {
		auto &children = expr.children;
		arg_types.reserve(children.size());
		arg_funcs.reserve(children.size());
		for (const auto &child : children) {
			arg_types.emplace_back(child->return_type);
			ListSegmentFunctions funcs;
			GetSegmentDataFunctions(funcs, arg_types.back());
			arg_funcs.emplace_back(funcs);
		}
		auto &order_bys = *expr.order_bys;
	}

	SortedAggregateBindData(const SortedAggregateBindData &other)
	    : function(other.function), arg_types(other.arg_types), arg_funcs(other.arg_funcs),
	      sort_types(other.sort_types), sort_funcs(other.sort_funcs), sorted_on_args(other.sorted_on_args),
	      threshold(other.threshold), external(other.external) {
		if (other.bind_info) {
			bind_info = other.bind_info->Copy();
		}
	}

	unique_ptr<FunctionData> Copy() const override {
		return make_uniq<SortedAggregateBindData>(*this);
	}

	bool Equals(const FunctionData &other_p) const override {
		auto &other = other_p.Cast<SortedAggregateBindData>();
		if (bind_info && other.bind_info) {
			if (!bind_info->Equals(*other.bind_info)) {
				return false;
			}
		} else if (bind_info || other.bind_info) {
			return false;
		}
		if (function != other.function) {
			return false;
		}
		return true;
	}

	AggregateFunction function;
	vector<LogicalType> arg_types;
	unique_ptr<FunctionData> bind_info;
	vector<ListSegmentFunctions> arg_funcs;

	vector<LogicalType> sort_types;
	vector<ListSegmentFunctions> sort_funcs;
	bool sorted_on_args;

	//! The sort flush threshold
	const idx_t threshold;
	const bool external;
};

struct SortedAggregateState {
	// Linked list equivalent of DataChunk
	using LinkedLists = vector<LinkedList>;
	using LinkedChunkFunctions = vector<ListSegmentFunctions>;

	//! Capacities of the various levels of buffering
	static const idx_t CHUNK_CAPACITY = STANDARD_VECTOR_SIZE;
	static const idx_t LIST_CAPACITY = MinValue<idx_t>(16, CHUNK_CAPACITY);

	SortedAggregateState() : count(0), nsel(0), offset(0) {
	}

	static inline void InitializeLinkedList(LinkedLists &linked, const vector<LogicalType> &types) {
		if (linked.empty() && !types.empty()) {
			linked.resize(types.size(), LinkedList());
		}
	}

	inline void InitializeLinkedLists(const SortedAggregateBindData &order_bind) {
		InitializeLinkedList(sort_linked, order_bind.sort_types);
		if (!order_bind.sorted_on_args) {
			InitializeLinkedList(arg_linked, order_bind.arg_types);
		}
	}

	static inline void InitializeChunk(unique_ptr<DataChunk> &chunk, const vector<LogicalType> &types) {
		if (!chunk && !types.empty()) {
			chunk = make_uniq<DataChunk>();
			chunk->Initialize(Allocator::DefaultAllocator(), types);
		}
	}

	void InitializeChunks(const SortedAggregateBindData &order_bind) {
		// Lazy instantiation of the buffer chunks
		InitializeChunk(sort_chunk, order_bind.sort_types);
		if (!order_bind.sorted_on_args) {
			InitializeChunk(arg_chunk, order_bind.arg_types);
		}
	}

	static inline void FlushLinkedList(const LinkedChunkFunctions &funcs, LinkedLists &linked, DataChunk &chunk) {
		idx_t total_count = 0;
		for (column_t i = 0; i < linked.size(); ++i) {
			funcs[i].BuildListVector(linked[i], chunk.data[i], total_count);
			chunk.SetCardinality(linked[i].total_capacity);
		}
	}

	void FlushLinkedLists(const SortedAggregateBindData &order_bind) {
		InitializeChunks(order_bind);
		FlushLinkedList(order_bind.sort_funcs, sort_linked, *sort_chunk);
		if (arg_chunk) {
			FlushLinkedList(order_bind.arg_funcs, arg_linked, *arg_chunk);
		}
	}

	void InitializeCollections(const SortedAggregateBindData &order_bind) {
	}

	void FlushChunks(const SortedAggregateBindData &order_bind) {
		D_ASSERT(sort_chunk);
		ordering->Append(*ordering_append, *sort_chunk);
		sort_chunk->Reset();

		if (arguments) {
			D_ASSERT(arg_chunk);
			arguments->Append(*arguments_append, *arg_chunk);
			arg_chunk->Reset();
		}
	}

	void Resize(const SortedAggregateBindData &order_bind, idx_t n) {
		count = n;

		//	Establish the current buffering
		if (count <= LIST_CAPACITY) {
			InitializeLinkedLists(order_bind);
		}

		if (count > LIST_CAPACITY && !sort_chunk && !ordering) {
			FlushLinkedLists(order_bind);
		}

		if (count > CHUNK_CAPACITY && !ordering) {
			InitializeCollections(order_bind);
			FlushChunks(order_bind);
		}
	}

	static void LinkedAppend(const LinkedChunkFunctions &functions, ArenaAllocator &allocator, DataChunk &input,
	                         LinkedLists &linked, SelectionVector &sel, idx_t nsel) {
		const auto count = input.size();
		for (column_t c = 0; c < input.ColumnCount(); ++c) {
			auto &func = functions[c];
			auto &linked_list = linked[c];
			RecursiveUnifiedVectorFormat input_data;
			Vector::RecursiveToUnifiedFormat(input.data[c], count, input_data);
			for (idx_t i = 0; i < nsel; ++i) {
				idx_t sidx = sel.get_index(i);
				func.AppendRow(allocator, linked_list, input_data, sidx);
			}
		}
	}

	static void LinkedAbsorb(LinkedLists &source, LinkedLists &target) {
		D_ASSERT(source.size() == target.size());
		for (column_t i = 0; i < source.size(); ++i) {
			auto &src = source[i];
			if (!src.total_capacity) {
				break;
			}

			auto &tgt = target[i];
			if (!tgt.total_capacity) {
				tgt = src;
			} else {
				// append the linked list
				tgt.last_segment->next = src.first_segment;
				tgt.last_segment = src.last_segment;
				tgt.total_capacity += src.total_capacity;
			}
		}
	}

	void Update(const AggregateInputData &aggr_input_data, DataChunk &sort_input, DataChunk &arg_input) {
		const auto &order_bind = aggr_input_data.bind_data->Cast<SortedAggregateBindData>();
		Resize(order_bind, count + sort_input.size());

		sel.Initialize(nullptr);
		nsel = sort_input.size();

		if (ordering) {
			//	Using collections
			ordering->Append(*ordering_append, sort_input);
			if (arguments) {
				arguments->Append(*arguments_append, arg_input);
			}
		} else if (sort_chunk) {
			//	Still using data chunks
			sort_chunk->Append(sort_input);
			if (arg_chunk) {
				arg_chunk->Append(arg_input);
			}
		} else {
			//	Still using linked lists
			LinkedAppend(order_bind.sort_funcs, aggr_input_data.allocator, sort_input, sort_linked, sel, nsel);
			if (!arg_linked.empty()) {
				LinkedAppend(order_bind.arg_funcs, aggr_input_data.allocator, arg_input, arg_linked, sel, nsel);
			}
		}

		nsel = 0;
		offset = 0;
	}

	void UpdateSlice(const AggregateInputData &aggr_input_data, DataChunk &sort_input, DataChunk &arg_input) {
		const auto &order_bind = aggr_input_data.bind_data->Cast<SortedAggregateBindData>();
		Resize(order_bind, count + nsel);

		if (ordering) {
			//	Using collections
			D_ASSERT(sort_chunk);
			sort_chunk->Slice(sort_input, sel, nsel);
			if (arg_chunk) {
				arg_chunk->Slice(arg_input, sel, nsel);
			}
			FlushChunks(order_bind);
		} else if (sort_chunk) {
			//	Still using data chunks
			sort_chunk->Append(sort_input, true, &sel, nsel);
			if (arg_chunk) {
				arg_chunk->Append(arg_input, true, &sel, nsel);
			}
		} else {
			//	Still using linked lists
			LinkedAppend(order_bind.sort_funcs, aggr_input_data.allocator, sort_input, sort_linked, sel, nsel);
			if (!arg_linked.empty()) {
				LinkedAppend(order_bind.arg_funcs, aggr_input_data.allocator, arg_input, arg_linked, sel, nsel);
			}
		}

		nsel = 0;
		offset = 0;
	}

	void Swap(SortedAggregateState &other) {
		std::swap(count, other.count);

		std::swap(arguments, other.arguments);
		std::swap(arguments_append, other.arguments_append);
		std::swap(ordering, other.ordering);
		std::swap(ordering_append, other.ordering_append);

		std::swap(sort_chunk, other.sort_chunk);
		std::swap(arg_chunk, other.arg_chunk);

		std::swap(sort_linked, other.sort_linked);
		std::swap(arg_linked, other.arg_linked);
	}

	void Absorb(const SortedAggregateBindData &order_bind, SortedAggregateState &other) {
		if (!other.count) {
			return;
		} else if (!count) {
			Swap(other);
			return;
		}

		//	Change to a state large enough for all the data
		Resize(order_bind, count + other.count);

		//	3x3 matrix.
		//	We can simplify the logic a bit because the target is already set for the final capacity
		if (!sort_chunk) {
			//	If the combined count is still linked lists,
			//	then just move the pointers.
			//	Note that this assumes ArenaAllocator is shared and the memory will not vanish under us.
			LinkedAbsorb(other.sort_linked, sort_linked);
			if (!arg_linked.empty()) {
				LinkedAbsorb(other.arg_linked, arg_linked);
			}

			other.Reset();
			return;
		}

		if (!other.sort_chunk) {
			other.FlushLinkedLists(order_bind);
		}

		if (!ordering) {
			//	Still using chunks, which means the source is using chunks or lists
			D_ASSERT(sort_chunk);
			D_ASSERT(other.sort_chunk);
			sort_chunk->Append(*other.sort_chunk);
			if (arg_chunk) {
				D_ASSERT(other.arg_chunk);
				arg_chunk->Append(*other.arg_chunk);
			}
		} else {
			// Using collections, so source could be using anything.
			if (other.ordering) {
				ordering->Combine(*other.ordering);
				if (arguments) {
					D_ASSERT(other.arguments);
					arguments->Combine(*other.arguments);
				}
			} else {
				ordering->Append(*other.sort_chunk);
				if (arguments) {
					D_ASSERT(other.arg_chunk);
					arguments->Append(*other.arg_chunk);
				}
			}
		}

		//	Free all memory as we have absorbed it.
		other.Reset();
	}

	void PrefixSortBuffer(DataChunk &prefixed) {
		for (column_t col_idx = 0; col_idx < sort_chunk->ColumnCount(); ++col_idx) {
			prefixed.data[col_idx + 1].Reference(sort_chunk->data[col_idx]);
		}
		prefixed.SetCardinality(*sort_chunk);
	}

	void Finalize(const SortedAggregateBindData &order_bind, DataChunk &prefixed, LocalSortState &local_sort) {
		if (arguments) {
			ColumnDataScanState sort_state;
			ordering->InitializeScan(sort_state);
			ColumnDataScanState arg_state;
			arguments->InitializeScan(arg_state);
			for (sort_chunk->Reset(); ordering->Scan(sort_state, *sort_chunk); sort_chunk->Reset()) {
				PrefixSortBuffer(prefixed);
				arg_chunk->Reset();
				arguments->Scan(arg_state, *arg_chunk);
				local_sort.SinkChunk(prefixed, *arg_chunk);
			}
		} else if (ordering) {
			ColumnDataScanState sort_state;
			ordering->InitializeScan(sort_state);
			for (sort_chunk->Reset(); ordering->Scan(sort_state, *sort_chunk); sort_chunk->Reset()) {
				PrefixSortBuffer(prefixed);
				local_sort.SinkChunk(prefixed, *sort_chunk);
			}
		} else {
			//	Force chunks so we can sort
			if (!sort_chunk) {
				FlushLinkedLists(order_bind);
			}

			PrefixSortBuffer(prefixed);
			if (arg_chunk) {
				local_sort.SinkChunk(prefixed, *arg_chunk);
			} else {
				local_sort.SinkChunk(prefixed, *sort_chunk);
			}
		}

		Reset();
	}

	void Reset() {
		//	Release all memory
		ordering.reset();
		arguments.reset();

		sort_chunk.reset();
		arg_chunk.reset();

		sort_linked.clear();
		arg_linked.clear();

		count = 0;
	}

	idx_t count;

	unique_ptr<ColumnDataCollection> arguments;
	unique_ptr<ColumnDataAppendState> arguments_append;
	unique_ptr<ColumnDataCollection> ordering;
	unique_ptr<ColumnDataAppendState> ordering_append;

	unique_ptr<DataChunk> sort_chunk;
	unique_ptr<DataChunk> arg_chunk;

	LinkedLists sort_linked;
	LinkedLists arg_linked;

	// Selection for scattering
	SelectionVector sel;
	idx_t nsel;
	idx_t offset;
};

struct SortedAggregateFunction {
	template <typename STATE>
	static void Initialize(STATE &state) {
		new (&state) STATE();
	}

	template <typename STATE>
	static void Destroy(STATE &state, AggregateInputData &aggr_input_data) {
		state.~STATE();
	}

	static void ProjectInputs(Vector inputs[], const SortedAggregateBindData &order_bind, idx_t input_count,
	                          idx_t count, DataChunk &arg_input, DataChunk &sort_input) {
		idx_t col = 0;

		if (!order_bind.sorted_on_args) {
			arg_input.InitializeEmpty(order_bind.arg_types);
			for (auto &dst : arg_input.data) {
				dst.Reference(inputs[col++]);
			}
			arg_input.SetCardinality(count);
		}

		sort_input.InitializeEmpty(order_bind.sort_types);
		for (auto &dst : sort_input.data) {
			dst.Reference(inputs[col++]);
		}
		sort_input.SetCardinality(count);
	}

	static void SimpleUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, data_ptr_t state,
	                         idx_t count) {
		const auto order_bind = aggr_input_data.bind_data->Cast<SortedAggregateBindData>();
		DataChunk arg_input;
		DataChunk sort_input;
		ProjectInputs(inputs, order_bind, input_count, count, arg_input, sort_input);

		const auto order_state = reinterpret_cast<SortedAggregateState *>(state);
		order_state->Update(aggr_input_data, sort_input, arg_input);
	}

	static void ScatterUpdate(Vector inputs[], AggregateInputData &aggr_input_data, idx_t input_count, Vector &states,
	                          idx_t count) {
		if (!count) {
			return;
		}

		// Append the arguments to the two sub-collections
		const auto &order_bind = aggr_input_data.bind_data->Cast<SortedAggregateBindData>();
		DataChunk arg_inputs;
		DataChunk sort_inputs;
		ProjectInputs(inputs, order_bind, input_count, count, arg_inputs, sort_inputs);

		// We have to scatter the chunks one at a time
		// so build a selection vector for each one.
		UnifiedVectorFormat svdata;
		states.ToUnifiedFormat(count, svdata);

		// Size the selection vector for each state.
		auto sdata = UnifiedVectorFormat::GetDataNoConst<SortedAggregateState *>(svdata);
		for (idx_t i = 0; i < count; ++i) {
			auto sidx = svdata.sel->get_index(i);
			auto order_state = sdata[sidx];
			order_state->nsel++;
		}

		// Build the selection vector for each state.
		vector<sel_t> sel_data(count);
		idx_t start = 0;
		for (idx_t i = 0; i < count; ++i) {
			auto sidx = svdata.sel->get_index(i);
			auto order_state = sdata[sidx];
			if (!order_state->offset) {
				//	First one
				order_state->offset = start;
				order_state->sel.Initialize(sel_data.data() + order_state->offset);
				start += order_state->nsel;
			}
			sel_data[order_state->offset++] = UnsafeNumericCast<sel_t>(sidx);
		}

		// Append nonempty slices to the arguments
		for (idx_t i = 0; i < count; ++i) {
			auto sidx = svdata.sel->get_index(i);
			auto order_state = sdata[sidx];
			if (!order_state->nsel) {
				continue;
			}

			order_state->UpdateSlice(aggr_input_data, sort_inputs, arg_inputs);
		}
	}

	template <class STATE, class OP>
	static void Combine(const STATE &source, STATE &target, AggregateInputData &aggr_input_data) {
		auto &order_bind = aggr_input_data.bind_data->Cast<SortedAggregateBindData>();
		auto &other = const_cast<STATE &>(source); // NOLINT: absorb explicitly allows destruction
		target.Absorb(order_bind, other);
	}

	static void Window(AggregateInputData &aggr_input_data, const WindowPartitionInput &partition,
	                   const_data_ptr_t g_state, data_ptr_t l_state, const SubFrames &subframes, Vector &result,
	                   idx_t rid) {
		throw InternalException("Sorted aggregates should not be generated for window clauses");
	}

	static void Finalize(Vector &states, AggregateInputData &aggr_input_data, Vector &result, idx_t count,
	                     const idx_t offset) {
	}
};

void FunctionBinder::BindSortedAggregate(ClientContext &context, BoundAggregateExpression &expr,
                                         const vector<unique_ptr<Expression>> &groups) {
	// Remove unnecessary ORDER BY clauses and return if nothing remains
	if (context.config.enable_optimizer) {
	}
	auto &bound_function = expr.function;
	auto &children = expr.children;
	auto &order_bys = *expr.order_bys;
	auto sorted_bind = make_uniq<SortedAggregateBindData>(context, expr);

	if (!sorted_bind->sorted_on_args) {
		// The arguments are the children plus the sort columns.
	}

	vector<LogicalType> arguments;
	arguments.reserve(children.size());
	for (const auto &child : children) {
		arguments.emplace_back(child->return_type);
	}

	// Replace the aggregate with the wrapper
	AggregateFunction ordered_aggregate(
	    bound_function.name, arguments, bound_function.return_type, AggregateFunction::StateSize<SortedAggregateState>,
	    AggregateFunction::StateInitialize<SortedAggregateState, SortedAggregateFunction>,
	    SortedAggregateFunction::ScatterUpdate,
	    AggregateFunction::StateCombine<SortedAggregateState, SortedAggregateFunction>,
	    SortedAggregateFunction::Finalize, bound_function.null_handling, SortedAggregateFunction::SimpleUpdate, nullptr,
	    AggregateFunction::StateDestroy<SortedAggregateState, SortedAggregateFunction>, nullptr,
	    SortedAggregateFunction::Window);

	expr.function = std::move(ordered_aggregate);
	expr.bind_info = std::move(sorted_bind);
	expr.order_bys.reset();
}

} // namespace duckdb
