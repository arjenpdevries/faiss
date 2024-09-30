#include "duckdb/execution/physical_plan_generator.hpp"

#include "duckdb/catalog/catalog_entry/scalar_function_catalog_entry.hpp"
#include "duckdb/common/types/column/column_data_collection.hpp"
#include "duckdb/execution/column_binding_resolver.hpp"
#include "duckdb/execution/operator/helper/physical_verify_vector.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/config.hpp"
#include "duckdb/main/query_profiler.hpp"
#include "duckdb/planner/expression/bound_function_expression.hpp"
#include "duckdb/planner/operator/list.hpp"
#include "duckdb/planner/operator/logical_extension_operator.hpp"

namespace duckdb {

class DependencyExtractor : public LogicalOperatorVisitor {
public:
	explicit DependencyExtractor(LogicalDependencyList &dependencies) : dependencies(dependencies) {
	}

protected:
	unique_ptr<Expression> VisitReplace(BoundFunctionExpression &expr, unique_ptr<Expression> *expr_ptr) {
		// extract dependencies from the bound function expression
		if (expr.function.dependency) {
			expr.function.dependency(expr, dependencies);
		}
		return nullptr;
	}

private:
	LogicalDependencyList &dependencies;
};

PhysicalPlanGenerator::PhysicalPlanGenerator(ClientContext &context) : context(context) {
}

PhysicalPlanGenerator::~PhysicalPlanGenerator() {
}

} // namespace duckdb
