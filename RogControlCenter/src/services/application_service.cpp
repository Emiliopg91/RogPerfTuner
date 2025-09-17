#include "../../include/services/application_service.hpp"

#include <signal.h>
#include <unistd.h>

#include "../../include/events/event_bus.hpp"
#include "../../include/gui/toaster.hpp"
#include "../../include/services/steam_service.hpp"
#include "../../include/translator/translator.hpp"
#include "../../include/utils/autoupdater.hpp"

ApplicationService::ApplicationService() {
	logger.info("Initializing ApplicationService");
	Logger::add_tab();

	logger.info("Copying helper binaries");
	FileUtils::copy(Constants::ASSETS_BIN_DIR, Constants::BIN_DIR);
	FileUtils::chmodRecursive(Constants::BIN_DIR, 0777);

	if (!Constants::DEV_MODE) {
		logger.info("Copying launcher script");
		FileUtils::mkdirs(Constants::BIN_APPLICATION_DIR);
		FileUtils::writeFileContent(Constants::LAUNCHER_FILE, buildLaunchFile());
	}

	logger.info("Copying icons");
	FileUtils::copy(Constants::ASSET_ICONS_DIR, Constants::ICONS_DIR);

	if (!FileUtils::exists(Constants::APP_DRAW_FILE)) {
		logger.info("Creating menu entry");
		FileUtils::writeFileContent(Constants::APP_DRAW_FILE, buildDesktopFile());
	}

	FileUtils::mkdirs(Constants::UPDATE_DIR);

	AutoUpdater::init(
		[this]() {
			applyUpdate();
		},

		[this]() {
			return steamService.getRunningGames().empty();
		});

	Logger::rem_tab();
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

void ApplicationService::applyUpdate() {
	logger.info("Applying update");
	Logger::add_tab();
	toaster.showToast(translator.translate("applying.update"));
	shell.run_command("nohup bash -c \"sleep 1 && " + Constants::LAUNCHER_FILE + "\" > /dev/null 2>&1 &");
	shutdown();
	Logger::rem_tab();
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

const std::string ApplicationService::buildLaunchFile() {
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