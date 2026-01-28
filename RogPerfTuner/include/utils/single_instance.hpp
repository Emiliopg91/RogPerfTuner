#pragma once

#include "models/others/loggable.hpp"
#include "models/others/singleton.hpp"

class SingleInstance : public Singleton<SingleInstance>, Loggable {
  public:
	/**
	 * @brief Acquires a lock to ensure only a single instance is running.
	 *
	 * This method enforces that only one instance of the application can run at a time.
	 * If another instance is already running, this call may block or fail depending on implementation.
	 */
	void acquire();

	bool killRunningInstance();

  private:
	friend class Singleton<SingleInstance>;
	SingleInstance() : Loggable("SingleInstance") {
	}
};
