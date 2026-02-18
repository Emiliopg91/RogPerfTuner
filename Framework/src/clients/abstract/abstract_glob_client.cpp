
#include "framework/clients/abstract/abstract_glob_client.hpp"

#include <glob.h>

std::vector<std::string> AbstractGlobClient::read(const int& head, const int& tail) {
	if (paths.empty()) {
		throw std::runtime_error("Globbed path " + glob_ + " doesn't exist");
	}

	std::vector<std::string> results;
	for (auto path : paths) {
		std::string cmd = "cat " + path;
		if (head > 0) {
			cmd += " | head -n" + std::to_string(head);
		}
		if (tail > 0) {
			cmd += " | tail -n" + std::to_string(tail);
		}

		results.emplace_back(sudo_ ? shell.run_elevated_command(cmd).stdout_str : shell.run_command(cmd).stdout_str);
	}

	return results;
}

void AbstractGlobClient::write(const std::string& content) {
	std::string cmd = "echo '" + content + "' | tee " + glob_;
	if (sudo_) {
		shell.run_elevated_command(cmd);
	} else {
		shell.run_command(cmd);
	}
}

bool AbstractGlobClient::available() {
	return !paths.empty();
}

AbstractGlobClient::AbstractGlobClient(const std::string& path, const std::string& name, const bool& sudo, const bool& required)
	: Loggable(name), sudo_(sudo), glob_(path) {
	glob_t glob_result;

	if (glob(path.c_str(), 0, nullptr, &glob_result) == 0) {
		for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
			paths.emplace_back(glob_result.gl_pathv[i]);
		}
	}
	globfree(&glob_result);

	if (paths.empty() && required) {
		throw std::runtime_error("Globbed path " + path + " doesn't exist");
	}
}
