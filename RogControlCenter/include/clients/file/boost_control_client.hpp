#pragma once

#include <memory>

#include "../../../include/clients/file/abstract/abstract_file_client.hpp"
#include "../../../include/utils/file_utils.hpp"
#include "../../models/others/singleton.hpp"

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
	void set_boost(bool& enabled);
	bool available();

  private:
	BoostControlClient();

	std::unique_ptr<BoostControlClientImpl> client;
	friend class Singleton<BoostControlClient>;
};