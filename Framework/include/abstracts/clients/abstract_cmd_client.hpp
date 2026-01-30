#pragma once

#include <string>

#include "abstracts/loggable.hpp"
#include "shell/shell.hpp"
#include "string_utils.hpp"

class AbstractCmdClient : public Loggable {
  protected:
	AbstractCmdClient(const std::string& command, const std::string& name, const bool& required = false)
		: Loggable(name), command_(command), available_(true) {
		if (!isCommandAvailable()) {
			if (required) {
				throw std::runtime_error("Command " + command_ + " not available");
			}

			logger->error("Command {} not available", command);
			available_ = false;
		}
	}

	CommandResult run_command(const std::string& args = "", const bool& check = true, const bool& sudo = false) {
		if (!available_) {
			throw std::runtime_error("Command " + command_ + " not available");
		}

		std::string cmd = StringUtils::trim(command_ + " " + args);

		CommandResult result;
		if (sudo) {
			result = shell.run_elevated_command(cmd, check);
		} else {
			result = shell.run_command(cmd, check);
		}

		return result;
	}

	bool isCommandAvailable() {
		std::string cmd = "which " + command_ + " >/dev/null 2>&1";
		int ret			= std::system(cmd.c_str());
		return ret == 0;
	}
	Shell& shell = Shell::getInstance();

  public:
	bool available() {
		return available_;
	}

  private:
	std::string command_;
	bool available_;
};
