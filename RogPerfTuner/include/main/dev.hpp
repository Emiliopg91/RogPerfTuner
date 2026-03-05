#pragma once

#include "framework/shell/shell.hpp"
#include "framework/utils/single_instance.hpp"
#include "utils/constants.hpp"
inline void runDevMode() {
	SingleInstance::getInstance().acquire(Constants::LOCK_FILE);

	LoggerProvider::initialize(Constants::LOG_FILE_NAME, Constants::LOG_DIR);

	Shell& shell = Shell::getInstance();
	shell.launch_in_terminal(Constants::DEV_MODE_FILE, true, true);
}