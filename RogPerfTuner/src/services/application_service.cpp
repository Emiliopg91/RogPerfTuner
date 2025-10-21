#include "../../include/services/application_service.hpp"

#include <signal.h>
#include <unistd.h>

#include <sstream>

#include "../../include/events/event_bus.hpp"
#include "../../include/utils/file_utils.hpp"

#ifdef DEV_MODE
#endif

ApplicationService::ApplicationService(std::optional<std::string> execPath) : Loggable("ApplicationService") {
	logger.info("Initializing ApplicationService");
	Logger::add_tab();

#ifndef DEV_MODE
	if (!FileUtils::exists(Constants::APP_DRAW_FILE)) {
		logger.info("Creating menu entry");
		Logger::add_tab();
		FileUtils::writeFileContent(Constants::APP_DRAW_FILE, buildDesktopFile());
		Logger::rem_tab();
	}
#endif

	if (execPath.has_value()) {
		logger.info("Creating helper scripts");
		Logger::add_tab();

		if (FileUtils::exists(Constants::BIN_DIR)) {
			FileUtils::remove(Constants::BIN_DIR);
		}
		FileUtils::mkdirs(Constants::BIN_DIR);

		FileUtils::mkdirs(Constants::BIN_PERFORMANCE_DIR);
		createScriptFile(Constants::NEXT_PROFILE_PATH, *execPath, "-p");

		FileUtils::mkdirs(Constants::BIN_RGB_DIR);
		createScriptFile(Constants::NEXT_EFFECT_PATH, *execPath, "-e");
		createScriptFile(Constants::INCREASE_BRIGHTNESS_PATH, *execPath, "-i");
		createScriptFile(Constants::DECREASE_BRIGHTNESS_PATH, *execPath, "-d");

		FileUtils::mkdirs(Constants::BIN_STEAM_DIR);
		createWrapperScriptFile(Constants::STEAM_WRAPPER_PATH, *execPath, "-r");
		createWrapperScriptFile(Constants::FLATPAK_WRAPPER_PATH, *execPath, "-f");

		FileUtils::chmodRecursive(Constants::BIN_DIR, 0777);

		Logger::rem_tab();
	}

	Logger::rem_tab();
}

const std::string ApplicationService::buildDesktopFile() {
	std::ostringstream ss;
	ss << "[Desktop Entry]\n"
	   << "Exec=rog-perf-tuner\n"
	   << "Icon=" << Constants::ASSET_ICON_45_FILE << "\n"
	   << "Name=" << Constants::APP_NAME << "\n"
	   << "WName=" << Constants::APP_NAME << "\n"
	   << "Comment=An utility to manage Asus Rog laptop performance\n"
	   << "Path=\n"
	   << "Terminal=False\n"
	   << "Type=Application\n"
	   << "Categories=Utility;\n";
	return ss.str();
}

void ApplicationService::createScriptFile(std::string path, std::string execPath, std::string option) {
	std::ostringstream script;
	script << "#!/bin/bash\n\n";
	script << "set -e\n\n";
	script << "\"" << execPath << "\" " << option << "\n";

	FileUtils::writeFileContent(path, script.str());
}

void ApplicationService::createWrapperScriptFile(std::string path, std::string wrp, std::string option) {
	std::ostringstream script;
	script << "#!/usr/bin/env bash\n\n";
	script << "set -e\n\n";
	script << "TARGET=\"" << wrp << "\"\n\n";
	script << "if [[ -f \"$TARGET\" && -x \"$TARGET\" ]]; then\n";
	script << "    exec \"$TARGET\" " << option << " \"$@\"\n";
	script << "else\n";
	script << "    exec \"$@\"\n";
	script << "fi\n";

	FileUtils::writeFileContent(path, script.str());
}

bool ApplicationService::isAutostartEnabled() {
	return FileUtils::exists(Constants::AUTOSTART_FILE);
}
void ApplicationService::setAutostart(bool enabled) {
	if (enabled) {
		if (!FileUtils::exists(Constants::AUTOSTART_FILE)) {
			FileUtils::writeFileContent(Constants::AUTOSTART_FILE, buildDesktopFile());
			logger.info("Autostart file '{}' written successfully", Constants::AUTOSTART_FILE);
		}
	} else {
		FileUtils::remove(Constants::AUTOSTART_FILE);
		logger.info("Autostart file '{}' deleted successfully", Constants::AUTOSTART_FILE);
	}
}

void ApplicationService::shutdown() {
	shuttingDown = true;
	logger.info("Starting application shutdown");
	Logger::add_tab();
	eventBus.emitApplicationStop();
	Logger::rem_tab();
	logger.info("Shutdown finished");
	kill(Constants::PID, SIGTERM);
}