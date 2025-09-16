#pragma once

#include <signal.h>

#include "../shell/shell.hpp"

class ProcessUtils {
  private:
	ProcessUtils() {
	}
	static Shell& getShell();

  public:
	static void sleep(long milliseconds);
	static void sendSignal(pid_t pid, int signal);
	static uint sendSignalToHierarchy(pid_t pid, int signal);
	static std::vector<pid_t> getAllPidsOfHierarchy(pid_t pid);
};