#pragma once

#include <string>

#include "logger/logger_level.hpp"

class Sink {
  public:
	virtual ~Sink() = default;

	virtual void write(const std::string&, LoggerLevel level) = 0;
};