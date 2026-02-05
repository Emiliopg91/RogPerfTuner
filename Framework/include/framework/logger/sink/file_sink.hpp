#pragma once

#include <fstream>
#include <string>

#include "framework/logger/sink/base/sink.hpp"

class FileSink : public Sink {
  public:
	explicit FileSink(const std::string& filename);
	~FileSink() override;

	void write(const std::string& message, LoggerLevel level) override;

  private:
	std::ofstream file;
};