#pragma once
#include <memory>

#include "framework/logger/logger_provider.hpp"

class Loggable {
  public:
  protected:
	Loggable(const std::string& name) : logger(LoggerProvider::getLogger(name)) {
	}

  protected:
	std::shared_ptr<Logger> logger;
};
