#pragma once

#include "../../models/others/singleton.hpp"
#include "../../models/steam/steam_game_config.hpp"
#include "abstract/abstract_unix_socket_client.hpp"

class RogPerfTunerClient : public AbstractUnixSocketClient, public Singleton<RogPerfTunerClient> {
  private:
	RogPerfTunerClient();
	friend class Singleton<RogPerfTunerClient>;

  public:
	void nextEffect();
	void decreaseBrightness();
	void increaseBrightness();

	SteamGameConfig getGameConfig(std::string steamId);
};