#pragma once

#include "../clients/websocket/steam_client.hpp"
#include "../models/hardware/gpu_brand.hpp"
#include "../models/settings/root_config.hpp"
#include "../models/steam/mangohud_level.hpp"
#include "../models/steam/wine_sync_option.hpp"
#include "./hardware_service.hpp"
#include "./open_rgb_service.hpp"
#include "./profile_service.hpp"
#include "RccCommons.hpp"

class SteamService {
  private:
	inline static std::string WRAPPER_PATH = Constants::BIN_DIR + "/steam/run";
	Logger logger{"SteamService"};
	std::map<unsigned int, std::string> runningGames;
	bool rccdcEnabled = false;
	std::thread installer;

	Shell& shell					 = Shell::getInstance();
	EventBus& eventBus				 = EventBus::getInstance();
	ProfileService& profileService	 = ProfileService::getInstance();
	OpenRgbService& openRgbService	 = OpenRgbService::getInstance();
	HardwareService& hardwareService = HardwareService::getInstance();

	SteamService();

	void onConnect(bool onBoot = false);
	void onDisconnect();
	void onGameLaunch(unsigned int gid, std::string name, int pid);
	void onFirstGameRun(unsigned int gid, std::string name, std::map<std::string, std::string> env);
	void onGameStop(unsigned int gid, std::string name);
	void setProfileForGames(bool onConnect = false);
	void installRccDC();
	void copyPlugin();
	bool checkIfRequiredInstallation();

  public:
	static SteamService& getInstance() {
		static SteamService instance;
		return instance;
	}

	const std::map<unsigned int, std::string>& getRunningGames() const;
	const std::map<std::string, GameEntry>& getGames() const;

	bool isRunning(const unsigned int& appid) const;
	bool metricsEnabled();
	const SteamGameConfig getConfiguration(const std::string& gid);

	const std::optional<GpuBrand> getPreferedGpu(const unsigned int& gid);
	void setPreferedGpu(const unsigned int& gid, const std::optional<GpuBrand>& gpu);

	bool isSteamDeck(const unsigned int& gid);
	void setSteamDeck(const unsigned int& gid, const bool& value);

	const MangoHudLevel getMetricsLevel(const unsigned int& gid);
	void setMetricsLevel(const unsigned int& gid, const MangoHudLevel& level);

	const WineSyncOption getWineSync(const unsigned int& gid);
	void setWineSync(const unsigned int& gid, const WineSyncOption& level);

	bool isProton(const unsigned int& gid);

	const std::string getEnvironment(const unsigned int& gid);
	void setEnvironment(const unsigned int& gid, const std::string& env);

	const std::string getParameters(const unsigned int& gid);
	void setParameters(const unsigned int& gid, const std::string& env);
};