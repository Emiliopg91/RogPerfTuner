#include "../../include/utils/single_instance.hpp"

#include <signal.h>

#include <fstream>
#include <iostream>

#include "../../include/utils/constants.hpp"
#include "../../include/utils/file_utils.hpp"

void SingleInstance::acquire() {
	killRunningInstance();

	std::ofstream out(Constants::LOCK_FILE);
	out << Constants::PID;
	out.close();
}

bool SingleInstance::killRunningInstance() {
	auto running = false;
	if (FileUtils::exists(Constants::LOCK_FILE)) {
		std::ifstream f(Constants::LOCK_FILE);
		pid_t pid;
		f >> pid;
		f.close();

		if (pid > 0 && kill(pid, 0) == 0) {
			std::cout << "Found instance with pid " << pid << ", killing..." << std::endl;
			kill(pid, SIGKILL);
			running = true;
		}
		FileUtils::remove(Constants::LOCK_FILE);
	}
	return running;
}