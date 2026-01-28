#pragma once

#include <string>

#include "models/others/loggable.hpp"
#include "utils/shell/shell.hpp"

class AbstractCmdClient : public Loggable {
  protected:
	AbstractCmdClient(const std::string& command, const std::string& name, const bool& required = true);

	CommandResult run_command(const std::string& args = "", const bool& check = true, const bool& sudo = false);

	bool isCommandAvailable();
	Shell& shell = Shell::getInstance();

  public:
	/**
	 * @brief Checks if the client is currently available for operations.
	 *
	 * @return true if the client is available; false otherwise.
	 */
	bool available();

  private:
	std::string command_;
	bool available_;
};
