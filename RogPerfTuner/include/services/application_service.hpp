#pragma once

#include <thread>

#include "../events/event_bus.hpp"
#include "../services/steam_service.hpp"
#include "../translator/translator.hpp"

class ApplicationService : public Singleton<ApplicationService>, Loggable {
  public:
	/**
	 * @brief Shuts down the application gracefully.
	 *
	 * This method initiates the shutdown process, ensuring that all necessary cleanup
	 * operations are performed before the application exits.
	 */
	void shutdown();

	/**
	 * @brief Checks if autostart is enabled for the application.
	 *
	 * @return true if autostart is enabled, false otherwise.
	 */
	bool isAutostartEnabled();

	/**
	 * @brief Enables or disables autostart for the application.
	 *
	 * @param enabled Set to true to enable autostart, or false to disable it.
	 */
	void setAutostart(bool enabled);

	/**
	 * @brief Initiates an asynchronous check for application updates.
	 *
	 * This method starts a background process that checks whether a newer version
	 * of the application is available. The update check is performed asynchronously
	 * to avoid blocking the main application thread.
	 */
	void startUpdateCheck();

  private:
	friend class Singleton<ApplicationService>;
	bool rccdcEnabled = false;
	bool shuttingDown = false;
	std::thread updateChecker;

	Shell& shell				 = Shell::getInstance();
	EventBus& eventBus			 = EventBus::getInstance();
	SteamService& steamService	 = SteamService::getInstance();
	Translator& translator		 = Translator::getInstance();
	Toaster& toaster			 = Toaster::getInstance();
	Configuration& configuration = Configuration::getInstance();

	const std::string buildDesktopFile();
	void createScriptFile(std::string path, std::string execPath, std::string option);
	void createWrapperScriptFile(std::string path, std::string execPath, std::string option);
	void lookForUpdates();
	std::optional<std::string> getLatestVersion();

	ApplicationService(std::optional<std::string> execPath);
};
