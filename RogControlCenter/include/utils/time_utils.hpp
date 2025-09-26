#pragma once

#include <chrono>

class TimeUtils {
  private:
	TimeUtils() {
	}

  public:
	/**
	 * @brief Gets the current high-resolution time point.
	 *
	 * @return The current time point.
	 */
	static std::chrono::time_point<std::chrono::high_resolution_clock> now();

	/**
	 * @brief Gets the time difference in milliseconds between two time points.
	 *
	 * @param t0 The start time point.
	 * @param t1 The end time point.
	 * @return The time difference in milliseconds.
	 */
	static long getTimeDiff(std::chrono::time_point<std::chrono::high_resolution_clock> t0,
							std::chrono::time_point<std::chrono::high_resolution_clock> t1);

	/**
	 * @brief Sleeps for the specified number of milliseconds.
	 *
	 * @param milliseconds The number of milliseconds to sleep.
	 */
	static void sleep(long milliseconds);
};