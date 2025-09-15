#include "../../include/utils/single_instance.hpp"

#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "../../include/utils/constants.hpp"
#include "../../include/utils/string_utils.hpp"

void SingleInstance::acquire() {
	namespace fs = std::filesystem;

	if (fs::exists(Constants::LOCK_FILE)) {
		std::ifstream f(Constants::LOCK_FILE);
		pid_t pid;
		f >> pid;
		f.close();

		if (pid > 0 && kill(pid, 0) == 0) {
			std::cout << "Application already running with pid " + StringUtils::trim(std::to_string(pid)) + ", killing..." << std::endl;
			kill(pid, SIGKILL);
		} else {
			fs::remove(Constants::LOCK_FILE);
		}
	}

	std::ofstream out(Constants::LOCK_FILE);
	out << Constants::PID;
	out.close();
}
