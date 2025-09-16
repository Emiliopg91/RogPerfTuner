#include "../../include/utils/process_utils.hpp"

#include <string>

#include "../../include/utils/string_utils.hpp"
#include "spdlog/fmt/bundled/format.h"

Shell& ProcessUtils::getShell() {
	static Shell& instance = Shell::getInstance();
	return instance;
}

void ProcessUtils::sendSignal(pid_t pid, int signal) {
	getShell().run_elevated_command(fmt::format("kill -{} {}", signal, pid));
}

uint ProcessUtils::sendSignalToHierarchy(pid_t pid, int signal) {
	return static_cast<uint>(
		std::stoul(getShell()
					   .run_elevated_command(fmt::format(
						   "pstree -p {} | grep -o '([0-9]\\+)' | grep -o '[0-9]\\+' | tee >(xargs -r kill -{} 2>/dev/null) | wc -l", pid, signal))
					   .stdout_str));
}

std::vector<pid_t> ProcessUtils::getAllPidsOfHierarchy(pid_t pid) {
	auto pids = StringUtils::splitLines(
		getShell().run_elevated_command(fmt::format("pstree -p {} | grep -o '([0-9]\\+)' | grep -o '[0-9]\\+'", pid)).stdout_str);

	std::vector<pid_t> result;
	for (auto p : pids) {
		result.emplace_back(std::stoi(p));
	}
	return result;
}