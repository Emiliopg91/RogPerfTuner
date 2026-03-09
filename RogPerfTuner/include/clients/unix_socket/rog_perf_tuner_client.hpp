#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_unix_socket_client.hpp"
#ifdef STEAM_SUPPORT
#include "models/steam/steam_game_config.hpp"
#endif

class RogPerfTunerClient : public AbstractUnixSocketClient, public Singleton<RogPerfTunerClient> {
  private:
	RogPerfTunerClient();
	friend class Singleton<RogPerfTunerClient>;

  public:
	void nextEffect();
	void decreaseBrightness();
	void increaseBrightness();

	void nextProfile();

	void showGui();

#ifdef STEAM_SUPPORT
	SteamGameConfig getGameConfig(std::string steamId);
#endif
};