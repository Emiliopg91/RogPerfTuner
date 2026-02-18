#pragma once

#include <string>

#include "framework/abstracts/loggable.hpp"
#include "framework/shell/shell.hpp"

class AbstractGlobClient : public Loggable {
  public:
	std::vector<std::string> read(const int& head = 0, const int& tail = 0);

	void write(const std::string& content);

	bool available();

  protected:
	AbstractGlobClient(const std::string& path, const std::string& name, const bool& sudo = false, const bool& required = true);
	std::string glob_;
	std::vector<std::string> paths;
	bool sudo_;
	Shell& shell = Shell::getInstance();
};
