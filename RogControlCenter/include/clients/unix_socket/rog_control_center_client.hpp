#pragma once

#include "../../models/hardware/rgb_brightness.hpp"
#include "../../models/performance/performance_profile.hpp"
#include "../../models/steam/steam_game_config.hpp"
#include "abstract/abstract_unix_socket_client.hpp"

class RogControlCenterClient : public AbstractUnixSocketClient, public Singleton<RogControlCenterClient> {
  private:
	RogControlCenterClient();

	friend class Singleton<RogControlCenterClient>;

  public:
	std::string nextEffect();
	RgbBrightness increaseBrightness();
	RgbBrightness decreaseBrightness();

	PerformanceProfile nextProfile();

	SteamGameConfig getGameConfiguration(std::string appId);
};