#include "../../include/services/application_service.hpp"

#include <signal.h>
#include <unistd.h>

#include "../../include/gui/toaster.hpp"
#include "../../include/services/steam_service.hpp"
#include "../../include/translator/translator.hpp"
#include "../../include/utils/autoupdater.hpp"
#include "RccCommons.hpp"

ApplicationService::ApplicationService() {
	logger.info("Initializing ApplicationService");
	logger.add_tab();

	FileUtils::copy(Constants::ASSETS_BIN_DIR, Constants::BIN_DIR);
	FileUtils::mkdirs(Constants::BIN_APPLICATION_DIR);

	std::string content = buildLaunchFile();
	FileUtils::writeFileContent(Constants::LAUNCHER_FILE, content);
	logger.debug("Launch file '" + Constants::LAUNCHER_FILE + "' written successfully");

	FileUtils::copy(Constants::ASSET_ICONS_DIR, Constants::ICONS_DIR);

	FileUtils::mkdirs(Constants::UPDATE_DIR);

	Shell::getInstance().run_elevated_command("chmod 777 " + Constants::BIN_DIR);

	content = buildDesktopFile();
	if (!FileUtils::exists(Constants::AUTOSTART_FILE)) {
		FileUtils::writeFileContent(Constants::AUTOSTART_FILE, content);
		logger.debug("Autostart file '" + Constants::AUTOSTART_FILE + "' written successfully");
	}
	if (!FileUtils::exists(Constants::APP_DRAW_FILE)) {
		FileUtils::writeFileContent(Constants::APP_DRAW_FILE, content);
		logger.debug("Menu entry file '" + Constants::APP_DRAW_FILE + "' written successfully");
	}

	AutoUpdater::getInstance([this]() { applyUpdate(); },

							 [this]() -> bool { return SteamService::getInstance().getRunningGames().empty(); });

	logger.rem_tab();
}

void ApplicationService::applyUpdate() {
	logger.info("Applying update");
	logger.add_tab();
	Toaster::getInstance().showToast(Translator::getInstance().translate("applying.update"));
	Shell::getInstance().run_command("nohup bash -c \"sleep 1 && " + Constants::LAUNCHER_FILE +
									 "\" > /dev/null 2>&1 &");
	shutdown();
	logger.rem_tab();
}

void ApplicationService::shutdown() {
	shuttingDown = true;
	logger.info("Starting application shutdown");
	logger.add_tab();
	EventBus::getInstance().emit_event(Events::APPLICATION_STOP);
	logger.rem_tab();
	logger.info("Shutdown finished");
	kill(getpid(), SIGTERM);
}