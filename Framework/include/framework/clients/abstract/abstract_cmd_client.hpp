#pragma once

#include <string>

#include "framework/abstracts/loggable.hpp"
#include "framework/shell/shell.hpp"

class AbstractCmdClient : public Loggable {
  protected:
	AbstractCmdClient(const std::string& command, const std::string& name, const bool& required = false);

	CommandResult run_command(const std::string& args = "", const bool& check = true, const bool& sudo = false);
	bool isCommandAvailable();
	Shell& shell = Shell::getInstance();

  public:
	bool available();

  private:
	std::string command_;
	bool available_;
};
