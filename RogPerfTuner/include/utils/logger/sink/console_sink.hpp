#pragma once

#include "base/sink.hpp"

class ConsoleSink : public Sink {
  public:
	ConsoleSink()			= default;
	~ConsoleSink() override = default;

	void write(std::string) override;
};