#include "RccCommons.hpp"

#include "../../include/gui/toaster.hpp"
#include "../../include/services/application_service.hpp"
#include "../../include/services/steam_service.hpp"
#include "../../include/translator/translator.hpp"
#include "../../include/utils/autoupdater.hpp"

ApplicationService::ApplicationService()
{
    logger.info("Initializing ApplicationService");
    logger.add_tab();

    if (!Constants::DEV_MODE)
    {
        std::string content = buildLaunchFile();
        FileUtils::writeFileContent(Constants::LAUNCHER_FILE, content);
        logger.debug("Launch file '" + Constants::LAUNCHER_FILE + "' written successfully");

        content = buildDesktopFile();
        if (FileUtils::exists(Constants::AUTOSTART_FILE))
        {
            FileUtils::writeFileContent(Constants::AUTOSTART_FILE, content);
            logger.debug("Autostart file '" + Constants::AUTOSTART_FILE + "' written successfully");
        }
        if (FileUtils::exists(Constants::APP_DRAW_FILE))
        {
            FileUtils::writeFileContent(Constants::APP_DRAW_FILE, content);
            logger.debug("Menu entry file '" + Constants::APP_DRAW_FILE + "' written successfully");
        }
    }

    AutoUpdater::getInstance([this]()
                             { applyUpdate(); },

                             [this]() -> bool
                             { return SteamService::getInstance().getRunningGames().empty(); });

    logger.rem_tab();
}

void ApplicationService::applyUpdate()
{
    logger.info("Applying update");
    logger.add_tab();
    Toaster::getInstance().showToast(Translator::getInstance().translate("applying.update"));
    Shell::getInstance().run_command("nohup bash -c \"sleep 1 && " + Constants::LAUNCHER_FILE + "\" > /dev/null 2>&1 &");
    exit(0);
    logger.rem_tab();
}
