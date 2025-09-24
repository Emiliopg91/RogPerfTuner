#pragma once

#include <signal.h>

#include <set>

#include "../shell/shell.hpp"

class ProcessUtils {
  private:
	ProcessUtils() {
	}
	static Shell& getShell();

  public:
	static void sendSignal(pid_t pid, int signal);
	static std::set<pid_t> sendSignalToHierarchy(pid_t pid, int signal);
	static std::set<pid_t> getAllPidsOfHierarchy(pid_t pid);
	static std::set<pid_t> reniceHierarchy(pid_t pid, int8_t value);
	static std::set<pid_t> ioniceHierarchy(pid_t pid, uint8_t cls, uint8_t value);
};