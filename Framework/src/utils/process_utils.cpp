#include "framework/utils/process_utils.hpp"

#include <cstdint>
#include <set>
#include <string>

#include "framework/utils/string_utils.hpp"

Shell& ProcessUtils::getShell() {
	static Shell& instance = Shell::getInstance();
	return instance;
}

void ProcessUtils::sendSignal(pid_t pid, int signal) {
	getShell().run_elevated_command("kill -" + std::to_string(signal) + " " + std::to_string(pid));
}

std::set<pid_t> ProcessUtils::getAllPidsOfHierarchy(pid_t pid) {
	auto pids = StringUtils::splitLines(
		getShell().run_elevated_command("pstree -p " + std::to_string(pid) + " | grep -o '([0-9]\\+)' | grep -o '[0-9]\\+'").stdout_str);

	std::set<pid_t> result;
	for (auto p : pids) {
		result.emplace(std::stoi(p));
	}
	return result;
}

std::set<pid_t> ProcessUtils::sendSignalToHierarchy(pid_t pid, int signal) {
	auto pids = getAllPidsOfHierarchy(pid);

	getShell().run_elevated_command("kill -" + std::to_string(signal) + " " + StringUtils::join(pids, " ") + " 2>/dev/null");

	return pids;
}

std::set<pid_t> ProcessUtils::reniceHierarchy(pid_t pid, int8_t value) {
	auto pids = getAllPidsOfHierarchy(pid);

	getShell().run_elevated_command("renice " + std::to_string(value) + " -p " + StringUtils::join(pids, " "));

	return pids;
}

std::set<pid_t> ProcessUtils::ioniceHierarchy(pid_t pid, uint8_t cls, uint8_t value) {
	auto pids = getAllPidsOfHierarchy(pid);

	getShell().run_elevated_command("ionice -c" + std::to_string(cls) + " -n" + std::to_string(value) + " -p " + StringUtils::join(pids, " "));

	return pids;
}

std::vector<std::string> ProcessUtils::getCmdLine(pid_t pid) {
	std::string path = "/proc/" + std::to_string(pid) + "/cmdline";
	std::ifstream file(path, std::ios::in | std::ios::binary);

	if (!file) {
		return {};
	}

	std::string raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	std::vector<std::string> args;
	std::string current;

	for (char c : raw) {
		if (c == '\0') {
			if (!current.empty()) {
				args.push_back(current);
				current.clear();
			}
		} else {
			current += c;
		}
	}

	// añadir último argumento si existe
	if (!current.empty()) {
		args.push_back(current);
	}

	return args;
}

std::unordered_map<std::string, std::string> ProcessUtils::getEnvironment(pid_t pid) {
	std::string path = "/proc/" + std::to_string(pid) + "/environ";
	std::ifstream file(path, std::ios::in | std::ios::binary);

	if (!file) {
		return {};
	}

	std::string raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	std::unordered_map<std::string, std::string> env;
	std::string current;

	for (char c : raw) {
		if (c == '\0') {
			if (!current.empty()) {
				auto pos = current.find('=');
				if (pos != std::string::npos) {
					env[current.substr(0, pos)] = current.substr(pos + 1);
				}
				current.clear();
			}
		} else {
			current += c;
		}
	}

	// añadir última variable si existe
	if (!current.empty()) {
		auto pos = current.find('=');
		if (pos != std::string::npos) {
			env[current.substr(0, pos)] = current.substr(pos + 1);
		}
	}

	return env;
}