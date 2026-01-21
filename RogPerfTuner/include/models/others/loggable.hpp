#pragma once
#include "../../utils/logger/logger_provider.hpp"

class Loggable {
  public:
  protected:
	Loggable(std::string name) : logger(*LoggerProvider::getLogger(name)) {
	}

  protected:
	Logger logger;
};
