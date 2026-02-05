#pragma once

#include "framework/logger/sink/base/sink.hpp"

class ConsoleSink : public Sink {
  public:
	ConsoleSink()			= default;
	~ConsoleSink() override = default;

	void write(const std::string&, LoggerLevel level) override;
};