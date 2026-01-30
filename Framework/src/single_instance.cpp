#include "single_instance.hpp"

#include <signal.h>

#include <fstream>
#include <iostream>

#include "file_utils.hpp"
#include "framework_constants.hpp"

void SingleInstance::acquire(std::string lockFile) {
	killRunningInstance(lockFile);

	std::ofstream out(lockFile);
	out << FrameworkConstants::PID;
	out.close();
}

bool SingleInstance::killRunningInstance(std::string lockFile) {
	auto running = false;
	if (FileUtils::exists(lockFile)) {
		std::ifstream f(lockFile);
		pid_t pid;
		f >> pid;
		f.close();

		if (pid > 0 && kill(pid, 0) == 0) {
			std::cout << "Found instance with pid " << pid << ", killing..." << std::endl;
			kill(pid, SIGKILL);
			running = true;
		}
		FileUtils::remove(lockFile);
	}
	return running;
}