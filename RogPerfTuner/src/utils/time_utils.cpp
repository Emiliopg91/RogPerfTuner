#include "../../include/utils/time_utils.hpp"

#include <thread>

std::chrono::time_point<std::chrono::high_resolution_clock> TimeUtils::now() {
	return std::chrono::high_resolution_clock::now();
}

long TimeUtils::getTimeDiff(std::chrono::time_point<std::chrono::high_resolution_clock> t0,
							std::chrono::time_point<std::chrono::high_resolution_clock> t1) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
}

void TimeUtils::sleep(long milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

int64_t TimeUtils::fileTimeToEpoch(const std::filesystem::file_time_type& ftime) {
	using namespace std::chrono;

	auto sctp = time_point_cast<system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + system_clock::now());

	return duration_cast<seconds>(sctp.time_since_epoch()).count();
}

std::string TimeUtils::format_seconds(long time_ms) {
	double seconds = time_ms / 1000.0;

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(3) << seconds;
	return oss.str();
}