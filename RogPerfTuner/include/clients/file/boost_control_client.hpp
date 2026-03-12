#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_file_client.hpp"

class BoostControlClient : public AbstractFileClient, public Singleton<BoostControlClient> {
  public:
	/**
	 * @brief Enables or disables the boost mode.
	 *
	 * @param enabled Reference to a boolean value. Set to true to enable boost mode, false to disable it.
	 */
	void set_boost(bool& enabled);
	/**
	 * @brief Checks if the control client is available.
	 *
	 * This function determines whether the control client can be accessed or is currently operational.
	 *
	 * @return true if the client is available; false otherwise.
	 */
	bool available();

  private:
	BoostControlClient();

	friend class Singleton<BoostControlClient>;
};