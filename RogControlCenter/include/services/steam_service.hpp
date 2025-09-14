#pragma once

#include "../clients/websocket/steam_client.hpp"
#include "../configuration/configuration.hpp"
#include "../models/steam/steam_game_config.hpp"
#include "./hardware_service.hpp"
#include "./open_rgb_service.hpp"
#include "./profile_service.hpp"

class SteamService {
  private:
	Logger logger{"SteamService"};
	std::unordered_map<unsigned int, std::string> runningGames;
	bool rccdcEnabled = false;
	std::thread installer;

	Shell& shell					 = Shell::getInstance();
	EventBus& eventBus				 = EventBus::getInstance();
	Configuration& configuration	 = Configuration::getInstance();
	ProfileService& profileService	 = ProfileService::getInstance();
	OpenRgbService& openRgbService	 = OpenRgbService::getInstance();
	HardwareService& hardwareService = HardwareService::getInstance();
	SteamClient& steamClient		 = SteamClient::getInstance();

	SteamService();

	void onConnect(bool onBoot = false);
	void onDisconnect();
	void onGameLaunch(unsigned int gid, std::string name, int pid);
	void onFirstGameRun(unsigned int gid, std::string name);
	void onGameStop(unsigned int gid, std::string name);
	void setProfileForGames(bool onConnect = false);
	void installRccDC();
	void copyPlugin();
	bool checkIfRequiredInstallation();
	std::string encodeAppId(uint32_t appid);

  public:
	static SteamService& getInstance() {
		static SteamService instance;
		return instance;
	}

	const std::unordered_map<unsigned int, std::string>& getRunningGames() const;
	const std::unordered_map<std::string, GameEntry>& getGames();

	bool isRunning(const unsigned int& appid) const;
	bool metricsEnabled();
	const SteamGameConfig getConfiguration(const std::string& gid);
	void saveGameConfig(uint gid, const GameEntry& entry);
	void launchGame(const std::string& id);
};