#pragma once

#include "../../models/others/singleton.hpp"
#include "./abstract/abstract_cmd_client.hpp"

class FlatpakClient : public AbstractCmdClient, public Singleton<FlatpakClient> {
  private:
	FlatpakClient() : AbstractCmdClient("flatpak", "FlatpakClient") {
	}
	friend class Singleton<FlatpakClient>;

  public:
	bool checkInstalled(const std::string& name, bool userland);
	bool install(const std::string& name, bool userland);
	bool override(const std::string& name, bool userland);
};