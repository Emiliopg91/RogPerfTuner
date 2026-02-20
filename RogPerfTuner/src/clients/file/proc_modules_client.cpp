
#include "clients/file/proc_modules_client.hpp"

#include "framework/utils/string_utils.hpp"
ProcModulesClient::ProcModulesClient() : AbstractFileClient("/proc/modules", "ProcModulesClient") {
}

bool ProcModulesClient::isModuleLoaded(std::string module) {
	auto content = read();
	auto lines	 = StringUtils::splitLines(content);

	for (auto& line : lines) {
		auto it = line.find(module + " ");
		if (it == 0) {
			return true;
		}
	}

	return false;
}