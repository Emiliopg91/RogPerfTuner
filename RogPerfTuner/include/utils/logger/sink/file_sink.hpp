#pragma once

#include <fstream>
#include <string>

#include "base/sink.hpp"

class FileSink : public Sink {
  public:
	explicit FileSink(const std::string& filename);
	~FileSink() override;

	void write(std::string, LoggerLevel level) override;

  private:
	std::ofstream file;
};