#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_unix_socket_client.hpp"
#include "models/steam/steam_game_config.hpp"

class RogPerfTunerClient : public AbstractUnixSocketClient, public Singleton<RogPerfTunerClient> {
  private:
	RogPerfTunerClient();
	friend class Singleton<RogPerfTunerClient>;

  public:
	std::string nextEffect();
	std::string decreaseBrightness();
	std::string increaseBrightness();

	std::string nextProfile();

	void showGui();

	SteamGameConfig getGameConfig(const std::string& steamId);
};