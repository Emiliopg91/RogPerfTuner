#pragma once

#include <signal.h>

#include <set>

#include "../shell/shell.hpp"

class ProcessUtils {
  private:
	ProcessUtils() {
	}
	static Shell& getShell();

  public:
	/**
	 * @brief Sends a signal to a process.
	 *
	 * @param pid The process ID to which the signal will be sent.
	 * @param signal The signal number to send (e.g., SIGTERM, SIGKILL).
	 */
	static void sendSignal(pid_t pid, int signal);

	/**
	 * @brief Sends a signal to a process and all its descendants.
	 *
	 * @param pid The root process ID.
	 * @param signal The signal number to send.
	 * @return A set of all process IDs that received the signal.
	 */
	static std::set<pid_t> sendSignalToHierarchy(pid_t pid, int signal);

	/**
	 * @brief Gets all process IDs in the hierarchy rooted at the given PID.
	 *
	 * @param pid The root process ID.
	 * @return A set of all descendant process IDs, including the root.
	 */
	static std::set<pid_t> getAllPidsOfHierarchy(pid_t pid);

	/**
	 * @brief Changes the nice value (priority) for a process hierarchy.
	 *
	 * @param pid The root process ID.
	 * @param value The nice value to set.
	 * @return A set of all process IDs that were reniced.
	 */
	static std::set<pid_t> reniceHierarchy(pid_t pid, int8_t value);

	/**
	 * @brief Changes the I/O scheduling class and priority for a process hierarchy.
	 *
	 * @param pid The root process ID.
	 * @param cls The I/O scheduling class.
	 * @param value The I/O priority value.
	 * @return A set of all process IDs that were ioniced.
	 */
	static std::set<pid_t> ioniceHierarchy(pid_t pid, uint8_t cls, uint8_t value);
};