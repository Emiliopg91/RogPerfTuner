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

  private:
	BoostControlClient();

	friend class Singleton<BoostControlClient>;
};