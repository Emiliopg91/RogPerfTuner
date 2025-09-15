#pragma once

#include "./abstract/abstract_cmd_client.hpp"

class AsusCtlClient : public AbstractCmdClient {
  private:
	AsusCtlClient() : AbstractCmdClient("asusctl", "AsusCtlClient") {
	}

  public:
	static AsusCtlClient& getInstance() {
		static AsusCtlClient instance;
		return instance;
	}

	void turnOffAura();
};