#pragma once
#include "../../utils/logger/logger.hpp"

class Loggable {
  public:
  protected:
	Loggable(std::string name) : logger(Logger(name)) {
	}

  protected:
	Logger logger;
};
