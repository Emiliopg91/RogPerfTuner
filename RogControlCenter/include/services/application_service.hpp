#pragma once

#include "../events/event_bus.hpp"
#include "../services/steam_service.hpp"
#include "../translator/translator.hpp"

class ApplicationService : public Singleton<ApplicationService>, Loggable {
  public:
	void applyUpdate();
	void shutdown();
	bool isAutostartEnabled();
	void setAutostart(bool enabled);

  private:
	friend class Singleton<ApplicationService>;
	bool rccdcEnabled = false;
	bool shuttingDown = false;

	Shell& shell			   = Shell::getInstance();
	EventBus& eventBus		   = EventBus::getInstance();
	SteamService& steamService = SteamService::getInstance();
	Translator& translator	   = Translator::getInstance();
	Toaster& toaster		   = Toaster::getInstance();

	const std::string buildDesktopFile();

	const std::string buildLaunchFile();

	ApplicationService();
};
