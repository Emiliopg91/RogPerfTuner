#pragma once

#include <string>

class NetUtils {
  public:
	/**
	 * @brief Checks if a network port is free (available).
	 *
	 * @param port The port number to check.
	 * @return true if the port is free, false otherwise.
	 */
	static bool isPortFree(int port);

	/**
	 * @brief Gets a random free network port.
	 *
	 * @return An available port number.
	 */
	static int getRandomFreePort();

	static void download(const std::string& url, const std::string& dst);

	static std::string fetch(const std::string& url);
};