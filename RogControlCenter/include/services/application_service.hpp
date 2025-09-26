#pragma once

#include "../events/event_bus.hpp"
#include "../services/steam_service.hpp"
#include "../translator/translator.hpp"

class ApplicationService : public Singleton<ApplicationService>, Loggable {
  public:
	/**
	 * @brief Applies an available update to the application.
	 *
	 * This method handles the process of updating the application to a newer version,
	 * if an update is available.
	 */
	void applyUpdate();

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

  private:
	friend class Singleton<ApplicationService>;
	bool rccdcEnabled = false;
	bool shuttingDown = false;

	Shell& shell				 = Shell::getInstance();
	EventBus& eventBus			 = EventBus::getInstance();
	SteamService& steamService	 = SteamService::getInstance();
	Translator& translator		 = Translator::getInstance();
	Toaster& toaster			 = Toaster::getInstance();
	Configuration& configuration = Configuration::getInstance();

	const std::string buildDesktopFile();

	const std::string buildLaunchFile();

	ApplicationService();
};
