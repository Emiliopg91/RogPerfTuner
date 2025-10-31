#include "../../include/services/application_service.hpp"

#include <httplib.h>
#include <signal.h>
#include <unistd.h>

#include <exception>
#include <optional>
#include <sstream>

#include "../../include/events/event_bus.hpp"
#include "../../include/models/others/semantic_version.hpp"
#include "../../include/utils/file_utils.hpp"
#include "../../include/utils/time_utils.hpp"

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
		this->execPath = *execPath;

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

#ifndef DEV_MODE
	if (!configuration.getConfiguration().application.enrolled) {
		if (enroll()) {
			configuration.getConfiguration().application.enrolled = true;
			configuration.saveConfig();
		}
	}
#endif

	Logger::rem_tab();
}

const std::string ApplicationService::buildDesktopFile() {
	std::ostringstream ss;
	ss << "[Desktop Entry]\n"
	   << "Exec=" << Constants::EXEC_NAME << "\n"
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

void ApplicationService::startUpdateCheck() {
	updateChecker = std::thread(&ApplicationService::lookForUpdates, this);
	updateChecker.detach();
}

void ApplicationService::lookForUpdates() {
	TimeUtils::sleep(5 * 1000);
	auto currentVersion = SemanticVersion::parse(Constants::APP_VERSION);

	httplib::SSLClient cli("aur.archlinux.org");
	bool found = false;

	while (true) {
		logger.info("Looking for update");
		Logger::add_tab();
		try {
			auto res = cli.Get("/rpc/?v=5&type=info&arg=" + Constants::EXEC_NAME);

			if (res && res->status == 200) {
				YAML::Node root	   = YAML::Load(res->body);
				auto version	   = root["results"][0]["Version"].as<std::string>();
				auto latestVersion = SemanticVersion::parse(version);

				if (latestVersion > currentVersion) {
					logger.info("New version available: {}", version);
					toaster.showToast(translator.translate("update.available", {{"version", version}}));
					eventBus.emitUpdateAvailable(version);
					found = true;
					break;
				}
			}
		} catch (std::exception& e) {
			logger.error("Error on update check: {}", e.what());
		}

		if (!found) {
			logger.info("No update found");
		}

		Logger::rem_tab();

		TimeUtils::sleep(60 * 60 * 1000);
	}
}

std::optional<std::string> ApplicationService::getLatestVersion() {
	return std::nullopt;
}

bool ApplicationService::enroll() {
	try {
		auto cli = httplib::SSLClient(Constants::COUNTER_API_HOST);
		auto res = cli.Get(Constants::COUNTER_API_URL + "/up", {{"Authorization", "Bearer " + Constants::COUNTER_API_SV}});
		return res && res->status == 200;
	} catch (std::exception& e) {
		return false;
	}
}

bool ApplicationService::unenroll() {
	try {
		auto cli = httplib::SSLClient(Constants::COUNTER_API_HOST);
		auto res = cli.Get(Constants::COUNTER_API_URL + "/down", {{"Authorization", "Bearer " + Constants::COUNTER_API_SV}});
		return res && res->status == 200;
	} catch (std::exception& e) {
		return false;
	}
}

void ApplicationService::applyUpdate() {
	const std::vector<const char*> helpers = std::vector({"paru", "yay"});

	std::optional<std::string> helper = std::nullopt;
	for (const auto& h : helpers) {
		const auto abs = shell.which(h);
		if (abs.has_value()) {
			helper = *abs;
			break;
		}
	}

	if (helper.has_value()) {
		PerformanceProfile p = PerformanceProfile::Enum::PERFORMANCE;
		performanceService.setPerformanceProfile(p, true, true, false);
		const auto result = shell.wait_for(shell.launch_in_terminal(fmt::format("{} -S {}", *helper, Constants::EXEC_NAME)));
		const auto w	  = *shell.which("nohup");
		shell.launch_process(w.data(), (char* const[]){const_cast<char*>("nohup"), const_cast<char*>("rog-perf-tuner"), nullptr}, environ);
	}
}