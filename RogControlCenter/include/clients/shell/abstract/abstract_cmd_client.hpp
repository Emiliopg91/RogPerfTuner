#pragma once

#include <string>

#include "../../../models/others/loggable.hpp"
#include "../../../shell/shell.hpp"

class AbstractCmdClient : public Loggable {
  protected:
	AbstractCmdClient(const std::string& command, const std::string& name, const bool& required = true);

	CommandResult run_command(const std::string& args = "", const bool& check = true, const bool& sudo = false);

  public:
	bool available();

  private:
	std::string command_;
	bool available_;
	Shell& shell = Shell::getInstance();

	bool isCommandAvailable();
};
