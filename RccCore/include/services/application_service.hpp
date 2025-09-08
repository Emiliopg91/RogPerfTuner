#pragma once

#include <sstream>

#include "../events/event_bus.hpp"
#include "../services/steam_service.hpp"
#include "../translator/translator.hpp"
#include "RccCommons.hpp"

class ApplicationService {
  public:
	static ApplicationService& getInstance() {
		static ApplicationService instance;
		return instance;
	}

	void applyUpdate();
	void shutdown();
	bool isAutostartEnabled();
	void setAutostart(bool enabled);

  private:
	Logger logger{"ApplicationService"};
	bool rccdcEnabled = false;
	bool shuttingDown = false;

	Shell& shell			   = Shell::getInstance();
	EventBus& eventBus		   = EventBus::getInstance();
	SteamService& steamService = SteamService::getInstance();
	Translator& translator	   = Translator::getInstance();

	const std::string buildDesktopFile() {
		std::ostringstream ss;
		ss << "[Desktop Entry]\n"
		   << "Exec=" << Constants::LAUNCHER_FILE << "\n"
		   << "Icon=" << Constants::ICON_45_FILE << "\n"
		   << "WName=" << Constants::APP_NAME << "\n"
		   << "Comment=An utility to manage Asus Rog laptop performance\n"
		   << "Path=\n"
		   << "Terminal=False\n"
		   << "Type=Application\n"
		   << "Categories=Utility;\n";
		return ss.str();
	}

	const std::string buildLaunchFile() {
		std::ostringstream ss;
		ss << "#!/bin/bash\n"
		   << "UPDATE_PATH=\"" << Constants::UPDATE_FILE << "\"\n"
		   << "APPIMAGE_PATH=\"" << Constants::APPIMAGE_FILE << "\"\n"
		   << "\n"
		   << "if [[ -f \"$UPDATE_PATH\" ]]; then\n"
		   << "  mv \"$UPDATE_PATH\" \"$APPIMAGE_PATH\"\n"
		   << "  chmod 755 \"$APPIMAGE_PATH\"\n"
		   << "  rm \"$UPDATE_PATH\"\n"
		   << "fi\n"
		   << Constants::APPIMAGE_FILE << "\n";
		return ss.str();
	}

	ApplicationService();
};
