#pragma once

#include <string>

#include "framework/abstracts/loggable.hpp"
#include "framework/shell/shell.hpp"

class AbstractCmdClient : public Loggable {
  protected:
	AbstractCmdClient(const std::string& command, const std::string& name, bool required = false);

	CommandResult run_command(const std::string& args = "", bool check = true, bool sudo = false);
	bool isCommandAvailable();
	Shell& shell = Shell::getInstance();

  public:
	bool available();

  private:
	std::string command_;
	bool available_;
};
