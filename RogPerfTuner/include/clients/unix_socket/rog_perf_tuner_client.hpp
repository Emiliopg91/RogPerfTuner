#pragma once

#include "clients/unix_socket/abstract/abstract_unix_socket_client.hpp"
#include "models/others/singleton.hpp"
#include "models/steam/steam_game_config.hpp"

class RogPerfTunerClient : public AbstractUnixSocketClient, public Singleton<RogPerfTunerClient> {
  private:
	RogPerfTunerClient();
	friend class Singleton<RogPerfTunerClient>;

  public:
	void nextEffect();
	void decreaseBrightness();
	void increaseBrightness();

	void nextProfile();

	SteamGameConfig getGameConfig(std::string steamId);
};