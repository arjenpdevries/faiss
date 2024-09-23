#include "duckdb/execution/operator/helper/physical_set.hpp"

#include "duckdb/common/string_util.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/database.hpp"

namespace duckdb {

void PhysicalSet::SetExtensionVariable(ClientContext &context, ExtensionOption &extension_option, const string &name,
                                       SetScope scope, const Value &value) {
	auto &config = DBConfig::GetConfig(context);
	auto &target_type = extension_option.type;
}

SourceResultType PhysicalSet::GetData(ExecutionContext &context, DataChunk &chunk, OperatorSourceInput &input) const {
	auto &config = DBConfig::GetConfig(context.client);
	// check if we are allowed to change the configuration option
	config.CheckLock(name);
	auto option = DBConfig::GetOptionByName(name);
	if (!option) {
		// check if this is an extra extension variable
		auto entry = config.extension_parameters.find(name);
		if (entry == config.extension_parameters.end()) {
			Catalog::AutoloadExtensionByConfigName(context.client, name);
			entry = config.extension_parameters.find(name);
			D_ASSERT(entry != config.extension_parameters.end());
		}
		SetExtensionVariable(context.client, entry->second, name, scope, value);
		return SourceResultType::FINISHED;
	}
	SetScope variable_scope = scope;
	if (variable_scope == SetScope::AUTOMATIC) {
		if (option->set_local) {
			variable_scope = SetScope::SESSION;
		} else {
			D_ASSERT(option->set_global);
			variable_scope = SetScope::GLOBAL;
		}
	}

	return SourceResultType::FINISHED;
}

} // namespace duckdb
