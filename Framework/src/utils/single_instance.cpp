#include "utils/single_instance.hpp"

#include <signal.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "utils/file_utils.hpp"

void SingleInstance::acquire(std::string lockFile) {
	killRunningInstance(lockFile);

	std::ofstream out(lockFile);
	out << getpid();
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