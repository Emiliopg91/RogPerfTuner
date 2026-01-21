#pragma once

#include <string>

class Sink {
  public:
	virtual ~Sink() = default;

	virtual void write(std::string) = 0;
};