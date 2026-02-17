#pragma once

#include <optional>

#include "clients/file/intel_rapl_uj_client.hpp"
#include "clients/shell/pip_client.hpp"
#include "clients/unix_socket/steam_client.hpp"
#include "models/steam/steam_game_config.hpp"
#include "services/hardware_service.hpp"
#include "services/open_rgb_service.hpp"
#include "services/performance_service.hpp"
#include "utils/configuration_wrapper.hpp"

class SteamService : public Singleton<SteamService>, Loggable {
  private:
	friend class Singleton<SteamService>;
	std::unordered_map<unsigned int, GameEntry> runningGames;
	bool rccdcEnabled = false;
	std::thread installer;
	std::optional<std::string> whichMangohud;
	std::optional<std::string> whichSystemdInhibit;

	Shell& shell						   = Shell::getInstance();
	IntelRaplUJClient& intelRaplUjClient   = IntelRaplUJClient::getInstance();
	EventBusWrapper& eventBus			   = EventBusWrapper::getInstance();
	ConfigurationWrapper& configuration	   = ConfigurationWrapper::getInstance();
	OpenRgbService& openRgbService		   = OpenRgbService::getInstance();
	HardwareService& hardwareService	   = HardwareService::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
	SteamSocketClient& steamClient		   = SteamSocketClient::getInstance();
	PipClient& pipClient				   = PipClient::getInstance();
	Translator& translator				   = Translator::getInstance();

	SteamService();

	void onConnect(bool onBoot = false);
	void onDisconnect();
	void onGameLaunch(unsigned int gid, std::string name, int pid);
	void onFirstGameRun(unsigned int gid, std::string name, bool proton);
	void onGameStop(unsigned int gid, std::string name);
	void setProfileForGames(bool onConnect = false);
	void installRccDC();
	void copyPlugin();
	void installPipDeps();
	std::optional<std::string> getImagePath(uint gid, std::string sufix);
	bool checkIfRequiredInstallation();

  public:
	/**
	 * @brief Gets the map of currently running games.
	 *
	 * @return A constant reference to a map of running games, keyed by their app IDs.
	 */
	const std::unordered_map<unsigned int, GameEntry>& getRunningGames() const;

	/**
	 * @brief Gets the map of all known games.
	 *
	 * @return A constant reference to a map of games, keyed by their string IDs.
	 */
	const std::map<uint, GameEntry>& getGames();

	/**
	 * @brief Checks if a game with the given app ID is currently running.
	 *
	 * @param appid The app ID of the game.
	 * @return true if the game is running, false otherwise.
	 */
	bool isRunning(const unsigned int& appid) const;

	/**
	 * @brief Checks if metrics collection is enabled.
	 *
	 * @return true if metrics are enabled, false otherwise.
	 */
	bool metricsEnabled();

	/**
	 * @brief Gets the configuration for a specific Steam game.
	 *
	 * @param gid The game ID as a string.
	 * @return The SteamGameConfig for the specified game.
	 */
	const SteamGameConfig getConfiguration(const std::string& gid);

	/**
	 * @brief Saves the configuration for a specific game.
	 *
	 * @param gid The game ID as an unsigned integer.
	 * @param entry The GameEntry containing the configuration to save.
	 */
	void saveGameConfig(uint gid, const GameEntry& entry);

	/**
	 * @brief Launches a game by its ID.
	 *
	 * @param id The game ID as a string.
	 */
	void launchGame(const std::string& id);

	std::optional<std::string> getIcon(uint gid);

	std::optional<std::string> getBanner(uint gid);
};