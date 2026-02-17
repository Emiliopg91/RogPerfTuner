#pragma once

#include <thread>

#include "framework/translator/translator.hpp"
#include "services/performance_service.hpp"
#include "services/steam_service.hpp"
#include "utils/event_bus_wrapper.hpp"

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

	bool isStartMinimized();

	void setStartMinimized(bool enabled);

  private:
	friend class Singleton<ApplicationService>;
	bool rccdcEnabled = false;
	bool shuttingDown = false;
	std::string execPath;
	std::thread updateChecker;

	Shell& shell						   = Shell::getInstance();
	EventBusWrapper& eventBus			   = EventBusWrapper::getInstance();
	SteamService& steamService			   = SteamService::getInstance();
	Translator& translator				   = Translator::getInstance();
	Toaster& toaster					   = Toaster::getInstance();
	ConfigurationWrapper& configuration	   = ConfigurationWrapper::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();

	const std::string buildDesktopFile();
	void createScriptFile(std::string path, std::string execPath, std::string option);
	void createWrapperScriptFile(std::string path, std::string execPath, std::string option);

	ApplicationService(std::optional<std::string> execPath);
};
