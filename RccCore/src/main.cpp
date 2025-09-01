#include <QApplication>
#include <unistd.h>
#include "httplib.h"

#include "RccCommons.hpp"

#include "../include/clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../include/servers/http/http_server.hpp"
#include "../include/services/application_service.hpp"
#include "../include/services/hardware_service.hpp"
#include "../include/services/open_rgb_service.hpp"
#include "../include/services/profile_service.hpp"
#include "../include/services/steam_service.hpp"
#include "../include/configuration/configuration.hpp"
#include "../include/gui/tray_icon.hpp"
#include "../include/gui/password_dialog.hpp"
#include "../include/gui/toaster.hpp"
#include "../include/translator/translator.hpp"
#include "../include/utils/constants.hpp"

int main(int argc, char **argv)
{
	std::strncpy(argv[0], "RogControlCenter", std::strlen(argv[0]));
	argv[0][std::strlen(argv[0])] = '\0';

	std::cout << "Running application " + Constants::EXECUTABLE_PATH << " with PID " << getpid() << std::endl;
	if (Constants::DEV_MODE)
	{
		std::cout << "Dev mode enabled" << std::endl;
	}
	std::cout << "Assets directory: " << Constants::ASSETS_DIR << std::endl;

	LoggerProvider::initialize(Constants::LOG_FILE_NAME, Constants::LOG_DIR);
	Logger logger{};

	logger.info("###################################################");
	logger.info("#            Starting RogControlCenter            #");
	logger.info("###################################################");
	logger.info("Version " + Constants::APP_VERSION);
	logger.info("Starting initialization");
	logger.add_tab();

	logger.info("Creating QT application");
	QApplication app(argc, argv);

	Translator::getInstance();

	Toaster::getInstance()
		.showToast(Translator::getInstance().translate("initializing"));

	if (Configuration::getInstance().getPassword().length() == 0)
	{
		PasswordDialog::getInstance().showDialog();
	}

	Shell::getInstance(Configuration::getInstance().getPassword());

	OpenRgbService::getInstance();
	HardwareService::getInstance();
	ProfileService::getInstance();
	ApplicationService::getInstance();
	SteamService::getInstance();

	HttpServer::getInstance();

	logger.info("Creating tray icon");
	TrayIcon::getInstance();

	logger.rem_tab();
	logger.info("Application ready");
	std::atexit([]()
				{
					Logger logger{};
					logger.info("Starting application shutdown");
					logger.add_tab();
					OpenRgbClient::getInstance().stop();
					HttpServer::getInstance().stop();
					logger.rem_tab();
					logger.info("Shutdown finished");
					kill(getpid(),SIGKILL); });

	return app.exec();
}
