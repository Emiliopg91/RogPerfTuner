
#include "framework/clients/abstract/abstract_file_client.hpp"

#include "framework/utils/file_utils.hpp"

std::string AbstractFileClient::read(int head, int tail) {
	if (!available_) {
		throw std::runtime_error("File " + path_ + " doesn't exist");
	}

	std::string cmd = "cat " + path_;
	if (head > 0) {
		cmd += " | head -n" + std::to_string(head);
	}
	if (tail > 0) {
		cmd += " | tail -n" + std::to_string(tail);
	}

	if (sudo_) {
		return shell.run_elevated_command(cmd).stdout_str;
	} else {
		return shell.run_command(cmd).stdout_str;
	}
}

void AbstractFileClient::write(const std::string& content) {
	std::string cmd = "echo '" + content + "' | tee " + path_;
	if (sudo_) {
		shell.run_elevated_command(cmd);
	} else {
		shell.run_command(cmd);
	}
}

bool AbstractFileClient::available() {
	return available_;
}

AbstractFileClient::AbstractFileClient(const std::string& path, const std::string& name, bool sudo, bool required)
	: Loggable(name), path_(path), sudo_(sudo) {
	available_ = FileUtils::exists(path);
	if (!available_ && required) {
		throw std::runtime_error("File " + path_ + " doesn't exist");
	}
}
