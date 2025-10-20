#include "../../../../include/clients/file/abstract/abstract_file_client.hpp"

std::string AbstractFileClient::read(const int& head, const int& tail) {
	if (!available_) {
		throw std::runtime_error(fmt::format("File {} doesn't exist", path_));
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

AbstractFileClient::AbstractFileClient(const std::string& path, const std::string& name, const bool& sudo, const bool& required)
	: Loggable(name), path_(path), sudo_(sudo) {
	available_ = shell.run_command("ls " + path).exit_code == 0;
	if (!available_) {
		if (required) {
			throw std::runtime_error(fmt::format("File {} doesn't exist", path_));
		}

		logger.error("File {} doesn't exist", path_);
	}
}