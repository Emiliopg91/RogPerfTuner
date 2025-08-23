#include <QApplication>
#include <unistd.h>

#include "../include/services/hardware_service.hpp"
#include "../include/services/profile_service.hpp"
#include "../include/configuration/configuration.hpp"
#include "../include/gui/tray_icon.hpp"
#include "../include/gui/password_dialog.hpp"
#include "../include/gui/toaster.hpp"
#include "../include/logger/logger.hpp"
#include "../include/shell/shell.hpp"
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

	LoggerProvider::initialize();
	Logger logger{};

	logger.info("###################################################");
	logger.info("#            Starting RogControlCenter            #");
	logger.info("###################################################");
	logger.info("Starting initialization");
	logger.add_tab();

	logger.info("Creating QT application");
	QApplication app(argc, argv);

	Translator::getInstance();

	if (Configuration::getInstance().getPassword().length() == 0)
	{
		PasswordDialog::getInstance().showDialog();
	}

	Shell::getInstance(Configuration::getInstance().getPassword());

	Toaster::getInstance()
		.showToast(Translator::getInstance().translate("initializing"));

	HardwareService::getInstance();
	ProfileService::getInstance();

	logger.info("Creating tray icon");
	TrayIcon::getInstance();

	logger.rem_tab();
	logger.info("Application ready");

	return app.exec();
}
