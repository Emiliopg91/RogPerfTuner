#pragma once

#include <chrono>

class TimeUtils {
  private:
	TimeUtils() {
	}

  public:
	static std::chrono::time_point<std::chrono::high_resolution_clock> now();

	static long getTimeDiff(std::chrono::time_point<std::chrono::high_resolution_clock> t0,
							std::chrono::time_point<std::chrono::high_resolution_clock> t1);
};