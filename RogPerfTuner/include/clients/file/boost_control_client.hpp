#pragma once

#include <memory>

#include "clients/file/abstract/abstract_file_client.hpp"
#include "models/others/singleton.hpp"

class BoostControlClientImpl : public AbstractFileClient {
  public:
	void set_boost(bool& enabled) {
		write(enabled ? on : off);
	}

	BoostControlClientImpl(const std::string& path, const std::string& on, const std::string& off)
		: AbstractFileClient(path, "BoostControlClientImpl", true), on(on), off(off) {
	}

  private:
	std::string on;
	std::string off;

	friend class BoostControlClient;
};

class BoostControlClient : public Singleton<BoostControlClient> {
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

	std::unique_ptr<BoostControlClientImpl> client;
	friend class Singleton<BoostControlClient>;
};