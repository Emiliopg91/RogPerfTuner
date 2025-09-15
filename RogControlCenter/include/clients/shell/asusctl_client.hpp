#pragma once

#include "../../models/others/singleton.hpp"
#include "./abstract/abstract_cmd_client.hpp"

class AsusCtlClient : public AbstractCmdClient, public Singleton<AsusCtlClient> {
  private:
	AsusCtlClient() : AbstractCmdClient("asusctl", "AsusCtlClient") {
	}
	friend class Singleton<AsusCtlClient>;

  public:
	void turnOffAura();
};