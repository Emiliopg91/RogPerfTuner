#include "../../include/services/application_service.hpp"

#include <signal.h>
#include <unistd.h>

#include <sstream>

#include "../../include/events/event_bus.hpp"
#include "../../include/utils/file_utils.hpp"

#ifndef IS_AURPKG
#include "../../include/gui/toaster.hpp"
#include "../../include/services/steam_service.hpp"
#include "../../include/translator/translator.hpp"
#include "../../include/utils/autoupdater.hpp"
#endif

ApplicationService::ApplicationService(std::optional<std::string> execPath) : Loggable("ApplicationService") {
	logger.info("Initializing ApplicationService");
	Logger::add_tab();

	logger.info("Copying icons");
	FileUtils::copy(Constants::ASSET_ICONS_DIR, Constants::ICONS_DIR);

	if (FileUtils::exists(Constants::AUTOSTART_FILE) &&
		configuration.getConfiguration().application.appimage != Constants::APPIMAGE_FILE.has_value()) {
		logger.info("Migrating autostart file");
		Logger::add_tab();
		setAutostart(false);
		setAutostart(true);
		Logger::rem_tab();
	}

	configuration.getConfiguration().application.appimage = Constants::APPIMAGE_FILE.has_value();
	configuration.saveConfig();

#ifndef IS_AURPKG
	if (Constants::APPIMAGE_FILE.has_value()) {
		logger.info("Running from AppImage");
		Logger::add_tab();

		if (!Constants::DEV_MODE) {
			logger.info("Copying launcher script");
			FileUtils::mkdirs(Constants::BIN_APPLICATION_DIR);
			std::ostringstream ss;
			ss << "#!/bin/bash\n"
			   << "UPDATE_PATH=\"" << Constants::UPDATE_FILE << "\"\n"
			   << "APPIMAGE_PATH=\"" << *Constants::APPIMAGE_FILE << "\"\n"
			   << "\n"
			   << "if [[ -f \"$UPDATE_PATH\" ]]; then\n"
			   << "  mv \"$UPDATE_PATH\" \"$APPIMAGE_PATH\"\n"
			   << "  chmod 755 \"$APPIMAGE_PATH\"\n"
			   << "  rm \"$UPDATE_PATH\"\n"
			   << "fi\n"
			   << *Constants::APPIMAGE_FILE << "\n";
			auto content = ss.str();
			FileUtils::writeFileContent(Constants::LAUNCHER_FILE, content);

			if (!FileUtils::exists(Constants::APP_DRAW_FILE)) {
				logger.info("Creating menu entry");
				FileUtils::writeFileContent(Constants::APP_DRAW_FILE, buildDesktopFile());
			}
		}

		AutoUpdater::init(
			[this]() {
				logger.info("Applying update");
				Logger::add_tab();
				toaster.showToast(translator.translate("applying.update"));
				shell.run_command("nohup bash -c \"sleep 1 && " + Constants::LAUNCHER_FILE + "\" > /dev/null 2>&1 &");
				shutdown();
				Logger::rem_tab();
			},

			[this]() {
				return steamService.getRunningGames().empty();
			});

		FileUtils::mkdirs(Constants::UPDATE_DIR);

		Logger::rem_tab();
	}
#else
	logger.info("Autoupdate not available for AUR package");
#endif

	logger.info("Copying helper binaries");
	FileUtils::copy(Constants::ASSETS_BIN_DIR, Constants::BIN_DIR);

	logger.info("Creating helper scripts");
	if (execPath.has_value()) {
		if (!FileUtils::exists(Constants::BIN_PERFORMANCE_DIR)) {
			FileUtils::mkdirs(Constants::BIN_PERFORMANCE_DIR);
		}
		createScriptFile(Constants::NEXT_PROFILE_PATH, *execPath, "-p");

		if (!FileUtils::exists(Constants::BIN_RGB_DIR)) {
			FileUtils::mkdirs(Constants::BIN_RGB_DIR);
		}
		createScriptFile(Constants::NEXT_EFFECT_PATH, *execPath, "-e");
		createScriptFile(Constants::INCREASE_BRIGHTNESS_PATH, *execPath, "-i");
		createScriptFile(Constants::DECREASE_BRIGHTNESS_PATH, *execPath, "-d");
	}

	if (FileUtils::exists(Constants::FLATPAK_WRAPPER_PATH)) {
		FileUtils::remove(Constants::FLATPAK_WRAPPER_PATH);
	}
	FileUtils::createSymlink(Constants::STEAM_WRAPPER_PATH, Constants::FLATPAK_WRAPPER_PATH);

	FileUtils::chmodRecursive(Constants::BIN_DIR, 0777);

	Logger::rem_tab();
}

void ApplicationService::createScriptFile(std::string path, std::string execPath, std::string option) {
	std::ostringstream script;
	script << "#!/bin/bash\n\n";
	script << "set -e\n\n";
	script << "\"" << execPath << "\" " << option << "\n";

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

const std::string ApplicationService::buildDesktopFile() {
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