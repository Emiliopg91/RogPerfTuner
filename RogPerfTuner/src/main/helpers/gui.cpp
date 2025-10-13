#include <linux/limits.h>

#include <QApplication>
#include <iostream>

#include "../../../include/configuration/configuration.hpp"
#include "../../../include/gui/password_dialog.hpp"
#include "../../../include/gui/toaster.hpp"
#include "../../../include/gui/tray_icon.hpp"
#include "../../../include/logger/logger_provider.hpp"
#include "../../../include/servers/socket_server.hpp"
#include "../../../include/services/application_service.hpp"
#include "../../../include/services/hardware_service.hpp"
#include "../../../include/services/open_rgb_service.hpp"
#include "../../../include/services/performance_service.hpp"
#include "../../../include/services/steam_service.hpp"
#include "../../../include/translator/translator.hpp"
#include "../../../include/utils/constants.hpp"
#include "../../../include/utils/single_instance.hpp"
#include "../../../include/utils/string_utils.hpp"

void terminateHandler() {
	std::cerr << "Unhandled exception detected\n";

	std::exception_ptr exptr = std::current_exception();
	if (exptr) {
		try {
			std::rethrow_exception(exptr);
		} catch (const std::exception& e) {
			std::cerr << "Exception type: " << typeid(e).name() << ", message: " << e.what() << "\n";
		} catch (...) {
		}
	} else {
		std::cerr << "Could not get exception information.\n";
	}

	std::abort();
}

std::string getExecutablePath(const char* argv0) {
	std::string input(argv0);

	if (input.find('/') == std::string::npos) {
		return input;
	}

	char resolved[PATH_MAX];
	if (realpath(argv0, resolved)) {
		return std::string(resolved);
	}

	return input;
}

int startGui(int argc, char** argv) {
	std::set_terminate(terminateHandler);

	auto execPath = getExecutablePath(argv[0]);

	std::strncpy(argv[0], (Constants::APP_NAME + " v" + Constants::APP_VERSION).c_str(), std::strlen(argv[0]));
	argv[0][std::strlen(argv[0])] = '\0';

	std::cout << "Running application with PID " << Constants::PID << std::endl;

	SingleInstance::getInstance().acquire();

	if (Constants::DEV_MODE) {
		std::cout << "Dev mode enabled" << std::endl;
	}
	std::cout << "Assets directory: " << Constants::ASSETS_DIR << std::endl;

	LoggerProvider::initialize(Constants::LOG_FILE_NAME, Constants::LOG_DIR);
	Logger logger{};

	std::string title = "Starting " + Constants::APP_NAME;
	title			  = StringUtils::leftPad(title, title.length() + (49 - title.length()) / 2);
	title			  = StringUtils::rightPad(title, 49);

	logger.info("###################################################");
	logger.info("#{}#", title);
	logger.info("###################################################");
	logger.info("Version {}", Constants::APP_VERSION);
	logger.info("Starting initialization");
	Logger::add_tab();

	Translator::getInstance();

	logger.info("Creating QT application");
	QApplication app(argc, argv);
	app.setDesktopFileName(QString::fromStdString(Constants::APP_DRAW_FILE));

	Toaster::getInstance().showToast(Translator::getInstance().translate("initializing"));

	Configuration& configuration = Configuration::getInstance();
	if (configuration.getPassword().length() == 0) {
		PasswordDialog::getInstance().showDialog();
	}

	Shell::init(configuration.getPassword());

	OpenRgbService::getInstance();
	HardwareService::getInstance();
	PerformanceService::getInstance();
	SteamService::getInstance();

	SocketServer::getInstance();

	ApplicationService::init(execPath);

	TrayIcon::init().show();

	Logger::rem_tab();
	logger.info("Application ready");

	return app.exec();
}