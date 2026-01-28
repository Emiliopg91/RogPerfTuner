#pragma once

#include "utils/logger/sink/base/sink.hpp"

class ConsoleSink : public Sink {
  public:
	ConsoleSink()			= default;
	~ConsoleSink() override = default;

	void write(std::string, LoggerLevel level) override;
};