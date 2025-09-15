#include "../../../../include/clients/shell/abstract/abstract_cmd_client.hpp"

#include <cstdlib>
#include <stdexcept>

#include "../../../../include/utils/string_utils.hpp"

AbstractCmdClient::AbstractCmdClient(const std::string& command, const std::string& name, const bool& required)
	: command_(command), available_(true) {
	logger_ = Logger{name};
	if (!isCommandAvailable()) {
		if (required) {
			throw std::runtime_error(fmt::format("Command {} not available", command_));
		}

		logger_.error("Command {} not available", command_);
		available_ = false;
	}
}

CommandResult AbstractCmdClient::run_command(const std::string& args, const bool& check, const bool& sudo) {
	if (!available_) {
		throw std::runtime_error(fmt::format("Command {} not available", command_));
	}

	std::string cmd = StringUtils::trim(fmt::format("{} {}", command_, args));

	CommandResult result;
	if (sudo) {
		result = shell.run_elevated_command(cmd, check);
	} else {
		result = shell.run_command(cmd, check);
	}

	return result;
}

bool AbstractCmdClient::available() {
	return available_;
}

bool AbstractCmdClient::isCommandAvailable() {
	std::string cmd = "which " + command_ + " >/dev/null 2>&1";
	int ret			= std::system(cmd.c_str());
	return ret == 0;
}