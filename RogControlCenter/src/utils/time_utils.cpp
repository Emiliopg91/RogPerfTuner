#include "../../include/utils/time_utils.hpp"

std::chrono::time_point<std::chrono::high_resolution_clock> TimeUtils::now() {
	return std::chrono::high_resolution_clock::now();
}

long TimeUtils::getTimeDiff(std::chrono::time_point<std::chrono::high_resolution_clock> t0,
							std::chrono::time_point<std::chrono::high_resolution_clock> t1) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
}