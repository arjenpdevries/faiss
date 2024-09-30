#include "duckdb/common/render_tree.hpp"

#include "duckdb/execution/operator/aggregate/physical_hash_aggregate.hpp"
#include "duckdb/execution/operator/join/physical_delim_join.hpp"
#include "duckdb/execution/operator/scan/physical_positional_scan.hpp"

namespace duckdb {

struct PipelineRenderNode {
	explicit PipelineRenderNode() {
	}

	unique_ptr<PipelineRenderNode> child;
};

} // namespace duckdb

namespace {

using duckdb::MaxValue;
using duckdb::PhysicalOperatorType;
using duckdb::PipelineRenderNode;
using duckdb::RenderTreeNode;

class TreeChildrenIterator {
public:
	static bool HasChildren() {
		return true;
	}
	static void Iterate() {
	}
};

} // namespace

namespace duckdb {

template <class T>
static void GetTreeWidthHeight(const T &op, idx_t &width, idx_t &height) {
	if (!TreeChildrenIterator::HasChildren()) {
		width = 1;
		height = 1;
		return;
	}
	width = 0;
	height = 0;

	height++;
}

static unique_ptr<RenderTreeNode> CreateNode(const LogicalOperator &op) {
	return nullptr;
}

static unique_ptr<RenderTreeNode> CreateNode(const PipelineRenderNode &op) {
	return nullptr;
}

static unique_ptr<RenderTreeNode> CreateNode(const ProfilingNode &op) {
	auto &info = op.GetProfilingInfo();
	InsertionOrderPreservingMap<string> extra_info;
	if (info.Enabled(MetricsType::EXTRA_INFO)) {
		extra_info = op.GetProfilingInfo().extra_info;
	}

	string node_name = "QUERY";
	if (op.depth > 0) {
		node_name = info.GetMetricAsString(MetricsType::OPERATOR_TYPE);
	}

	auto result = make_uniq<RenderTreeNode>(node_name, extra_info);
	if (info.Enabled(MetricsType::OPERATOR_CARDINALITY)) {
		result->extra_text[RenderTreeNode::CARDINALITY] = info.GetMetricAsString(MetricsType::OPERATOR_CARDINALITY);
	}
	if (info.Enabled(MetricsType::OPERATOR_TIMING)) {
		string timing = StringUtil::Format("%.2f", info.metrics.at(MetricsType::OPERATOR_TIMING).GetValue<double>());
		result->extra_text[RenderTreeNode::TIMING] = timing + "s";
	}
	return result;
}

template <class T>
static idx_t CreateTreeRecursive(RenderTree &result, const T &op, idx_t x, idx_t y) {
	auto node = CreateNode(op);

	if (!TreeChildrenIterator::HasChildren()) {
		result.SetNode(x, y, std::move(node));
		return 1;
	}
	idx_t width = 0;
	// render the children of this node
	result.SetNode(x, y, std::move(node));
	return width;
}

template <class T>
static unique_ptr<RenderTree> CreateTree(const T &op) {
	idx_t width, height;
	GetTreeWidthHeight<T>(op, width, height);

	auto result = make_uniq<RenderTree>(width, height);

	// now fill in the tree
	CreateTreeRecursive<T>(*result, op, 0, 0);
	return result;
}

RenderTree::RenderTree(idx_t width_p, idx_t height_p) : width(width_p), height(height_p) {
	nodes = make_uniq_array<unique_ptr<RenderTreeNode>>((width + 1) * (height + 1));
}

optional_ptr<RenderTreeNode> RenderTree::GetNode(idx_t x, idx_t y) {
	if (x >= width || y >= height) {
		return nullptr;
	}
	return nodes[GetPosition(x, y)].get();
}

bool RenderTree::HasNode(idx_t x, idx_t y) {
	if (x >= width || y >= height) {
		return false;
	}
	return nodes[GetPosition(x, y)].get() != nullptr;
}

idx_t RenderTree::GetPosition(idx_t x, idx_t y) {
	return y * width + x;
}

void RenderTree::SetNode(idx_t x, idx_t y, unique_ptr<RenderTreeNode> node) {
	nodes[GetPosition(x, y)] = std::move(node);
}

unique_ptr<RenderTree> RenderTree::CreateRenderTree(const ProfilingNode &op) {
	return CreateTree<ProfilingNode>(op);
}

void RenderTree::SanitizeKeyNames() {
	for (idx_t i = 0; i < width * height; i++) {
		if (!nodes[i]) {
			continue;
		}
		InsertionOrderPreservingMap<string> new_map;
		for (auto &entry : nodes[i]->extra_text) {
			auto key = entry.first;
			if (StringUtil::StartsWith(key, "__")) {
				key = StringUtil::Replace(key, "__", "");
				key = StringUtil::Replace(key, "_", " ");
				key = StringUtil::Title(key);
			}
			auto &value = entry.second;
			new_map.insert(make_pair(key, value));
		}
		nodes[i]->extra_text = std::move(new_map);
	}
}

unique_ptr<RenderTree> RenderTree::CreateRenderTree(const Pipeline &pipeline) {
	auto operators = pipeline.GetOperators();
	D_ASSERT(!operators.empty());
	unique_ptr<PipelineRenderNode> node;
	for (auto &op : operators) {
		auto new_node = make_uniq<PipelineRenderNode>();
		new_node->child = std::move(node);
		node = std::move(new_node);
	}
	return CreateTree<PipelineRenderNode>(*node);
}

} // namespace duckdb
