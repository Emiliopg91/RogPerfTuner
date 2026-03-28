#pragma once

#include <syslog.h>

#include <fstream>
#include <string>

#include "framework/logger/sink/base/sink.hpp"
#include "framework/models/logger_level.hpp"

class SystemSink : public Sink {
  public:
	explicit SystemSink(std::string appName);
	~SystemSink() override;

	void write(const std::string& message, LoggerLevel level) override;

  private:
	std::ofstream file;
};