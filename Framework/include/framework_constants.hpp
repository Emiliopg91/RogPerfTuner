#pragma once

#include <sys/types.h>
#include <unistd.h>

#include <string>

class FrameworkConstants {
  public:
	inline static const std::string DEFAULT_LOGGER_NAME = "Default";
	inline static const std::string HOME_DIR			= std::getenv("HOME");
	inline static const pid_t PID						= getpid();
};
