#pragma once

#include "framework/abstracts/loggable.hpp"
#include "framework/abstracts/singleton.hpp"

class SingleInstance : public Singleton<SingleInstance>, Loggable {
  public:
	/**
	 * @brief Acquires a lock to ensure only a single instance is running.
	 *
	 * This method enforces that only one instance of the application can run at a time.
	 * If another instance is already running, this call may block or fail depending on implementation.
	 */
	void acquire(const std::string& lockFile);

	bool killRunningInstance(const std::string& lockFile);

  private:
	friend class Singleton<SingleInstance>;
	SingleInstance() : Loggable("SingleInstance") {
	}
};
