#include "../../include/utils/process_utils.hpp"

#include <chrono>
#include <thread>

void ProcessUtils::sleep(long milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}