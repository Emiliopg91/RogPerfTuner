#include "../../include/utils/process_utils.hpp"

#include <cstdint>
#include <set>
#include <string>

#include "../../include/utils/string_utils.hpp"

Shell& ProcessUtils::getShell() {
	static Shell& instance = Shell::getInstance();
	return instance;
}

void ProcessUtils::sendSignal(pid_t pid, int signal) {
	getShell().run_elevated_command(fmt::format("kill -{} {}", signal, pid));
}

std::set<pid_t> ProcessUtils::getAllPidsOfHierarchy(pid_t pid) {
	auto pids = StringUtils::splitLines(
		getShell().run_elevated_command(fmt::format("pstree -p {} | grep -o '([0-9]\\+)' | grep -o '[0-9]\\+'", pid)).stdout_str);

	std::set<pid_t> result;
	for (auto p : pids) {
		result.emplace(std::stoi(p));
	}
	return result;
}

std::set<pid_t> ProcessUtils::sendSignalToHierarchy(pid_t pid, int signal) {
	auto pids = getAllPidsOfHierarchy(pid);

	getShell().run_elevated_command(fmt::format("kill -{} {} 2>/dev/null", signal, StringUtils::join(pids, " ")));

	return pids;
}

std::set<pid_t> ProcessUtils::reniceHierarchy(pid_t pid, int8_t value) {
	auto pids = getAllPidsOfHierarchy(pid);

	getShell().run_elevated_command(fmt::format("renice {} -p {}", value, StringUtils::join(pids, " ")));

	return pids;
}

std::set<pid_t> ProcessUtils::ioniceHierarchy(pid_t pid, uint8_t cls, uint8_t value) {
	auto pids = getAllPidsOfHierarchy(pid);

	getShell().run_elevated_command(fmt::format("ionice -c{} -n{} -p {}", cls, value, StringUtils::join(pids, " ")));

	return pids;
}