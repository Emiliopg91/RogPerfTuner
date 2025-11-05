#include "../../include/services/application_service.hpp"

#include <signal.h>
#include <unistd.h>
#include <yaml-cpp/node/parse.h>

#include <cstddef>
#include <exception>
#include <optional>
#include <sstream>
#include <vector>

#include "../../include/events/event_bus.hpp"
#include "../../include/models/others/release_entry.hpp"
#include "../../include/models/others/semantic_version.hpp"
#include "../../include/utils/file_utils.hpp"
#include "../../include/utils/net_utils.hpp"
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

		const auto currentVersion = configuration.getConfiguration().application.currentVersion;
		if (currentVersion != Constants::APP_VERSION) {
			configuration.getConfiguration().application.previousVersion = configuration.getConfiguration().application.currentVersion;
			configuration.getConfiguration().application.currentVersion	 = Constants::APP_VERSION;
			configuration.saveConfig();
		}
	}

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

#ifdef AUR_HELPER
void ApplicationService::startUpdateCheck() {
	updateChecker = std::thread(&ApplicationService::lookForUpdates, this);
	updateChecker.detach();
}

void ApplicationService::lookForUpdates() {
	TimeUtils::sleep(5 * 1000);
	auto currentVersion = SemanticVersion::parse(Constants::APP_VERSION);

	bool found = false;

	while (true) {
		logger.info("Looking for update");
		Logger::add_tab();
		try {
			auto version	   = aurHelperClient.getVersion(Constants::EXEC_NAME);
			auto latestVersion = SemanticVersion::parse(version);

			if (latestVersion > currentVersion) {
				logger.info("New version available: {}", version);
				toaster.showToast(translator.translate("update.available", {{"version", version}}));
				eventBus.emitUpdateAvailable(version);
				found = true;
				break;
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

void ApplicationService::applyUpdate() {
	logger.info("Applying update...");
	Logger::add_tab();

	PerformanceProfile p = PerformanceProfile::Enum::PERFORMANCE;
	performanceService.setPerformanceProfile(p, true, true, false);

	logger.info("Launching update command...");
	Logger::add_tab();
	aurHelperClient.install(Constants::EXEC_NAME);
	Logger::rem_tab();

	logger.info("Relaunching application...");
	const auto abs_path = *shell.which(Constants::EXEC_NAME);
	shell.launch_process(abs_path.c_str(), (char* const[]){(char*)abs_path.c_str(), NULL}, environ);
	Logger::rem_tab();

	Logger::rem_tab();
}
#endif

std::optional<std::string> ApplicationService::getChangeLog() {
	try {
		const auto changelogYml = NetUtils::fetch(Constants::CHANGELOG_URL);
		YAML::Node root			= YAML::Load(changelogYml);

		if (!root.IsSequence()) {
			logger.error("Invalid changelog format");
			return std::nullopt;
		}

		const SemanticVersion currentSV = SemanticVersion::parse(Constants::APP_VERSION);

		std::vector<std::string> fixes;
		std::vector<std::string> features;

		std::vector<ReleaseEntry> releases = root.as<std::vector<ReleaseEntry>>();
		for (const auto& release : releases) {
			const auto version	 = release.version;
			const auto versionSV = SemanticVersion::parse(version);

			if (versionSV > currentSV) {
				for (const auto& f : release.features) {
					features.emplace_back(f);
				}

				for (const auto& f : release.fixes) {
					fixes.emplace_back(f);
				}
			}
		}

		std::ostringstream sb;
		sb << fmt::format("<h1>Changelog from {} (current version) to {} (latest version)</h1>\n", Constants::APP_VERSION, releases[0].version);
		sb << "\t<div style=\"margin-left:20px\">\n";

		sb << "\t\t<h2 style=\"margin-bottom: 0px;\">New features</h2>\n";
		sb << "\t\t<ul style=\"margin-top: 5px;\">\n";
		for (const auto& f : features) {
			sb << fmt::format("\t\t\t<li>{}</li>\n", f);
		}
		sb << "\t\t</ul>\n";

		sb << "\t\t<h2 style=\"margin-bottom: 0px;\">Fixes and improvements</h2>\n";
		sb << "\t\t<ul style=\"margin-top: 5px;\">\n";
		for (const auto& f : fixes) {
			sb << fmt::format("\t\t\t<li>{}</li>\n", f);
		}
		sb << "\t\t</ul>\n";

		sb << "\t</div>\n";

		return sb.str();
	} catch (std::exception& e) {
		return std::nullopt;
	}
}