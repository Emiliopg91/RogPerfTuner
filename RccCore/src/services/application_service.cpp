#include "../../include/services/application_service.hpp"

#include <signal.h>
#include <unistd.h>

#include "../../include/events/events.hpp"
#include "../../include/gui/toaster.hpp"
#include "../../include/services/steam_service.hpp"
#include "../../include/translator/translator.hpp"
#include "../../include/utils/autoupdater.hpp"

ApplicationService::ApplicationService() {
	logger.info("Initializing ApplicationService");
	Logger::add_tab();

	FileUtils::copy(Constants::ASSETS_BIN_DIR, Constants::BIN_DIR);
	FileUtils::mkdirs(Constants::BIN_APPLICATION_DIR);

	FileUtils::writeFileContent(Constants::LAUNCHER_FILE, buildLaunchFile());
	logger.debug("Launch file '" + Constants::LAUNCHER_FILE + "' written successfully");

	FileUtils::copy(Constants::ASSET_ICONS_DIR, Constants::ICONS_DIR);

	FileUtils::mkdirs(Constants::UPDATE_DIR);

	FileUtils::chmodRecursive(Constants::BIN_DIR, 0777);

	if (!FileUtils::exists(Constants::APP_DRAW_FILE)) {
		FileUtils::writeFileContent(Constants::APP_DRAW_FILE, buildDesktopFile());
		logger.debug("Menu entry file '" + Constants::APP_DRAW_FILE + "' written successfully");
	}

	AutoUpdater::getInstance([this]() { applyUpdate(); },

							 [this]() -> bool { return steamService.getRunningGames().empty(); });

	Logger::rem_tab();
}

bool ApplicationService::isAutostartEnabled() {
	return FileUtils::exists(Constants::AUTOSTART_FILE);
}
void ApplicationService::setAutostart(bool enabled) {
	if (enabled) {
		if (!FileUtils::exists(Constants::AUTOSTART_FILE)) {
			FileUtils::writeFileContent(Constants::AUTOSTART_FILE, buildDesktopFile());
			logger.info("Autostart file '" + Constants::AUTOSTART_FILE + "' written successfully");
		}
	} else {
		FileUtils::remove(Constants::AUTOSTART_FILE);
		logger.info("Autostart file '" + Constants::AUTOSTART_FILE + "' deleted successfully");
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
	eventBus.emit_event(Events::APPLICATION_STOP);
	Logger::rem_tab();
	logger.info("Shutdown finished");
	kill(getpid(), SIGTERM);
}