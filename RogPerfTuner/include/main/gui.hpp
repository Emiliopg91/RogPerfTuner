#pragma once

#include <linux/limits.h>

#include <QApplication>
#include <iostream>

#include "../configuration/configuration.hpp"
#include "../gui/password_dialog.hpp"
#include "../gui/toaster.hpp"
#include "../gui/tray_icon.hpp"
#include "../logger/logger_provider.hpp"
#include "../servers/socket_server.hpp"
#include "../services/application_service.hpp"
#include "../services/hardware_service.hpp"
#include "../services/open_rgb_service.hpp"
#include "../services/performance_service.hpp"
#include "../services/steam_service.hpp"
#include "../translator/translator.hpp"
#include "../utils/constants.hpp"
#include "../utils/single_instance.hpp"
#include "../utils/string_utils.hpp"
#include "../utils/time_utils.hpp"

inline void terminateHandler() {
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

inline std::string getExecutablePath(const char* argv0) {
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

inline int startGui(int argc, char** argv) {
	auto t0 = TimeUtils::now();
	std::set_terminate(terminateHandler);

	auto execPath = getExecutablePath(argv[0]);

	std::strncpy(argv[0], (Constants::APP_NAME + " v" + Constants::APP_VERSION).c_str(), std::strlen(argv[0]));
	argv[0][std::strlen(argv[0])] = '\0';

	SingleInstance::getInstance().acquire();
	std::cout << "Running application with PID " << Constants::PID << std::endl;

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
#ifdef DEV_MODE
	logger.info("Starting initialization in dev mode");
#else
	logger.info("Starting initialization");
#endif
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

	auto& applicationService = ApplicationService::init(execPath);

	TrayIcon::init().show();

	Logger::rem_tab();

	auto t1 = TimeUtils::now();
	logger.info("Application ready after {} seconds", TimeUtils::getTimeDiff(t0, t1) / 1000.0);

#ifdef AUR_HELPER
	applicationService.startUpdateCheck();
#endif

	logger.error(*applicationService.getChangeLog());

	return app.exec();
}

inline int killInstance() {
	return SingleInstance::getInstance().killRunningInstance() ? 0 : 1;
}