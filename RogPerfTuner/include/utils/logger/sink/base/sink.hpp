#pragma once

#include <string>

#include "models/others/logger_level.hpp"

class Sink {
  public:
	virtual ~Sink() = default;

	virtual void write(std::string, LoggerLevel level) = 0;
};