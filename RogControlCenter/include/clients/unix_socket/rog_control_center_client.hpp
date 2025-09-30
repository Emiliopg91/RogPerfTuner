#pragma once

#include "../../models/others/singleton.hpp"
#include "../../models/steam/steam_game_config.hpp"
#include "abstract/abstract_unix_socket_client.hpp"

class RogControlCenterClient : public AbstractUnixSocketClient, public Singleton<RogControlCenterClient> {
  private:
	RogControlCenterClient();
	friend class Singleton<RogControlCenterClient>;

  public:
	void nextEffect();
	void decreaseBrightness();
	void increaseBrightness();

	SteamGameConfig getGameConfig(std::string steamId);
};