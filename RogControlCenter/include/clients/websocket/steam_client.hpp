#pragma once

#include "../../models/others/singleton.hpp"
#include "../../models/steam/steam_game_details.hpp"
#include "abstract/abstract_websocket_client.hpp"

class SteamClient : public AbstractWebsocketClient, public Singleton<SteamClient> {
  private:
	SteamClient();
	friend class Singleton<SteamClient>;

  public:
	void onGameLaunch(CallbackWithParams&& callback);

	void onGameStop(CallbackWithParams&& callback);

	const std::vector<SteamGameDetails> getAppsDetails(const std::vector<unsigned int>& appIds);

	void setLaunchOptions(const int& appid, const std::string& launchOpts);

	std::string getIcon(const int& appid);
};