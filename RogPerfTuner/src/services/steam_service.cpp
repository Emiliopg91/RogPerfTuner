#include "../../include/services/steam_service.hpp"

#include <QApplication>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <thread>

#include "../../include/gui/game_config_dialog.hpp"
#include "../../include/utils/configuration/configuration.hpp"
#include "../../include/utils/events/event_bus.hpp"
#ifndef DEV_MODE
#include "../../include/gui/yes_no_dialog.hpp"
#endif
#include "../../include/models/others/semantic_version.hpp"
#include "../../include/models/steam/steam_game_details.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/services/performance_service.hpp"
#include "../../include/utils/file_utils.hpp"
#include "../../include/utils/net_utils.hpp"
#include "../../include/utils/process_utils.hpp"
#include "../../include/utils/string_utils.hpp"
#include "../../include/utils/time_utils.hpp"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"

bool SteamService::metricsEnabled() {
	return whichMangohud.has_value();
}

bool SteamService::isRunning(const unsigned int& appid) const {
	for (const auto& [key, val] : runningGames) {
		if (key == appid) {
			return true;
		}
	}
	return false;
}

const std::unordered_map<unsigned int, GameEntry>& SteamService::getRunningGames() const {
	return runningGames;
}

const std::unordered_map<std::string, GameEntry>& SteamService::getGames() {
	return configuration.getConfiguration().games;
}

SteamService::SteamService() : Loggable("SteamService") {
	logger.info("Initializing SteamService");
	Logger::add_tab();

	whichSystemdInhibit = shell.which("systemd-inhibit");

	whichMangohud = shell.which("mangohud");
	if (whichMangohud.has_value()) {
		if (FileUtils::exists(std::string("/sys/class/powercap/intel-rapl\\:0/energy_uj"))) {
			logger.info("Enabling CPU power drain...");
			shell.run_elevated_command("chmod o+r /sys/class/powercap/intel-rapl\\:0/energy_uj", false);
		}
	}

	if (!FileUtils::exists(Constants::LOGOS_DIR)) {
		FileUtils::mkdirs(Constants::LOGOS_DIR);
	}

	if (steamClient.connected()) {
		onConnect(true);
	}

	installRccDC();

	steamClient.onConnect([this]() {
		onConnect();
	});

	steamClient.onDisconnect([this]() {
		onDisconnect();
	});

	steamClient.onGameLaunch([this](CallbackParam data) {
		auto id	  = static_cast<uint32_t>(std::any_cast<long long>(data[0]));
		auto name = std::any_cast<std::string>(data[1]);
		auto pid  = static_cast<int>(std::any_cast<long long>(data[2]));

		onGameLaunch(id, name, pid);
	});

	steamClient.onGameStop([this](CallbackParam data) {
		auto id	  = static_cast<uint32_t>(std::any_cast<long long>(data[0]));
		auto name = std::any_cast<std::string>(data[1]);

		onGameStop(id, name);
	});

	Logger::rem_tab();
}

void SteamService::onFirstGameRun(unsigned int gid, std::string name) {
	logger.info("Configuring game");
	Logger::add_tab();

	SteamGameDetails details = steamClient.getAppsDetails({gid})[0];
	auto launch_opts		 = details.launch_opts;

	std::string env, args, wrappers;

	const std::string marker = "%command%";

	size_t pos = launch_opts.find(marker);
	if (pos != std::string::npos) {
		env = launch_opts.substr(0, pos);
		env.erase(0, env.find_first_not_of(" \t\n\r"));
		env.erase(env.find_last_not_of(" \t\n\r") + 1);
		launch_opts = launch_opts.substr(pos + marker.size());
		launch_opts.erase(0, launch_opts.find_first_not_of(" \t\n\r"));
		launch_opts.erase(launch_opts.find_last_not_of(" \t\n\r") + 1);
	}

	if (!launch_opts.empty()) {
		args = launch_opts;
	}

	{
		std::istringstream iss(env);
		std::string token;
		std::string pure_env;

		while (iss >> token) {
			if (token.find('=') != std::string::npos) {
				if (!pure_env.empty()) {
					pure_env += " ";
				}
				pure_env += token;
			} else {
				if (token != Constants::STEAM_WRAPPER_PATH) {
					if (!wrappers.empty()) {
						wrappers += " ";
					}
					wrappers += token;
				}
			}
		}

		env = pure_env;
	}

	GameEntry entry{args,
					env,
					std::nullopt,
					std::nullopt,
					MangoHudLevel::Enum::NO_DISPLAY,
					name,
					details.is_shortcut ? std::optional<std::string>{encodeAppId(gid)} : std::nullopt,
					!details.compat_tool.empty(),
					ComputerType::Enum::COMPUTER,
					WineSyncOption::Enum::AUTO,
					wrappers};
	configuration.getConfiguration().games[std::to_string(gid)] = entry;
	configuration.saveConfig();

	auto userIds = FileUtils::listDirectory(Constants::STEAM_USERDATA_PATH);
	for (const auto& userId : userIds) {
		auto path = Constants::STEAM_USERDATA_PATH + "/" + userId + "/config/grid/" + std::to_string(gid) + "_logo.png";
		if (FileUtils::exists(path)) {
			FileUtils::copy(path, Constants::LOGOS_DIR + "/" + std::to_string(gid) + "_logo.png");
			break;
		}
	}
	for (const auto& userId : userIds) {
		auto path = Constants::STEAM_USERDATA_PATH + "/" + userId + "/config/grid/" + std::to_string(gid) + "_hero.png";
		if (FileUtils::exists(path)) {
			FileUtils::copy(path, Constants::LOGOS_DIR + "/" + std::to_string(gid) + "_hero.png");
			break;
		}
	}

	if (!details.icon_hash.empty()) {
		auto icon = getIcon(gid);
		if (!icon.has_value()) {
			try {
				NetUtils::download("https://cdn.cloudflare.steamstatic.com/steamcommunity/public/images/apps/" + std::to_string(gid) + "/" +
									   details.icon_hash + ".jpg",
								   Constants::LOGOS_DIR + "/" + std::to_string(gid) + "_logo.jpg");
			} catch (std::exception& e) {
			}
		}
	}
	auto banner = getBanner(gid);
	if (!banner.has_value()) {
		try {
			NetUtils::download("https://cdn.cloudflare.steamstatic.com/steam/apps/" + std::to_string(gid) + "/header.jpg",
							   Constants::LOGOS_DIR + "/" + std::to_string(gid) + "_hero.jpg");
		} catch (std::exception& e) {
		}
	}

	steamClient.setLaunchOptions(gid, Constants::STEAM_WRAPPER_PATH + " %command%");

	QMetaObject::invokeMethod(
		qApp,
		[this, gid]() {
			GameConfigDialog(gid, true).showDialog();
		},
		Qt::QueuedConnection);

	Logger::rem_tab();
	Logger::rem_tab();
}

void SteamService::saveGameConfig(uint gid, const GameEntry& entry) {
	logger.info("Saving configuration for '" + entry.name + "' (" + std::to_string(gid) + ")");
	Logger::add_tab();

	configuration.getConfiguration().games[std::to_string(gid)] = entry;
	configuration.saveConfig();

	Logger::rem_tab();
}

void SteamService::launchGame(const std::string& id) {
	logger.info("Launching game with id " + id + "...");
	Logger::add_tab();

	shell.run_command("steam steam://rungameid/" + id);

	Logger::rem_tab();
}

bool SteamService::checkIfRequiredInstallation() {
	YAML::Node node = YAML::LoadFile(Constants::RCCDC_PACKAGE_FILE);

	SemanticVersion vA = SemanticVersion::parse(Constants::PLUGIN_VERSION);
	SemanticVersion vR = SemanticVersion::parse(node["version"].as<std::string>());

	return vA > vR;
}

void SteamService::installRccDC() {
	try {
		if (FileUtils::exists(Constants::DECKY_SERVICE_PATH)) {
			if (!FileUtils::exists(Constants::RCCDC_PATH)) {
#ifndef DEV_MODE
				if (!configuration.getConfiguration().application.askedInstallRccdc &&
					YesNoDialog::showDialog(translator.translate("enable.decky.integration.title"),
											translator.translate("enable.decky.integration.body"))) {
#endif
					logger.info("Installing plugin for first time");
					Logger::add_tab();
					installPipDeps();
					copyPlugin();
					Logger::rem_tab();
#ifndef DEV_MODE
				}
#endif
				configuration.getConfiguration().application.askedInstallRccdc = true;
				configuration.saveConfig();
			} else {
				if (checkIfRequiredInstallation()) {
					logger.info("Updating Decky plugin");
					Logger::add_tab();
					installPipDeps();
					copyPlugin();
					Logger::rem_tab();
				} else {
					logger.info("Plugin up to date");
				}
				configuration.getConfiguration().application.askedInstallRccdc = true;
				configuration.saveConfig();
			}
			rccdcEnabled = true;
		} else {
			logger.warn("No Decky installation found, skipping plugin installation");
			rccdcEnabled = false;
		}
	} catch (std::exception& e) {
		logger.error("Error while installing RCCDeckyCompanion plugin: " + std::string(e.what()));
	}
}

void SteamService::copyPlugin() {
	logger.info("Copying plugin");
	Logger::add_tab();
	installer = std::thread([this]() {
		FileUtils::copy(Constants::RCCDC_ASSET_PATH, Constants::USER_PLUGIN_DIR);

		std::vector<std::string> commands{"if [[ -d \"" + Constants::RCCDC_PATH + "\" ]]; then rm -R \"" + Constants::RCCDC_PATH + "\"; fi",
										  "chmod 777 \"" + Constants::PLUGINS_FOLDER + "/..\"",
										  "mv \"" + Constants::USER_PLUGIN_DIR + "\" \"" + Constants::RCCDC_PATH + "\"",
										  "systemctl restart plugin_loader.service"};

		for (auto cmd : commands) {
			shell.run_elevated_command(cmd);
		}
	});
	Logger::rem_tab();
}

void SteamService::installPipDeps() {
	auto depStr = StringUtils::join(Constants::RCCDC_REQUIRED_PIP, " ");
	logger.info("Installing PIP dependencies " + depStr);
	Logger::add_tab();
	pipClient.installPackage(depStr);
	Logger::rem_tab();
}

void SteamService::onGameLaunch(unsigned int gid, std::string name, int pid) {
	logger.info("Launched '" + name + "' (" + std::to_string(gid) + ") with PID " + std::to_string(pid));
	Logger::add_tab();

	auto it = configuration.getConfiguration().games.find(std::to_string(gid));
	if (it == configuration.getConfiguration().games.end()) {
		logger.info("Game not configured");
		Logger::add_tab();

		logger.info("Stopping process...");

		std::set<pid_t> signaled, newSignaled;
		do {
			signaled	= newSignaled;
			newSignaled = ProcessUtils::sendSignalToHierarchy(pid, SIGSTOP);

			logger.debug("Stopped " + std::to_string(newSignaled.size()) + " processes, before " + std::to_string(signaled.size()));

			TimeUtils::sleep(100);
		} while (signaled != newSignaled);
		logger.debug("Killed " + std::to_string(ProcessUtils::sendSignalToHierarchy(pid, SIGKILL).size()) + " processes");

		Logger::rem_tab();

		std::thread([this, gid, name]() {
			onFirstGameRun(gid, name);
		}).detach();
	} else if (runningGames.find(gid) == runningGames.end()) {
		runningGames[gid] = GameEntry(it->second);
		performanceService.renice(pid);
		setProfileForGames();

		eventBus.emitGameEvent(runningGames.size());
	}
	Logger::rem_tab();
	Logger::rem_tab();
}

void SteamService::onGameStop(unsigned int gid, std::string name) {
	auto it = runningGames.find(gid);
	if (it != runningGames.end()) {
		logger.info("Stopped '" + name + "' (" + std::to_string(gid) + ")");
		runningGames.erase(gid);
		Logger::add_tab();

		if (it->second.scheduler.has_value() && performanceService.getCurrentScheduler() == it->second.scheduler) {
			performanceService.setScheduler(std::nullopt);
		}

		setProfileForGames();

		eventBus.emitGameEvent(runningGames.size());
		Logger::rem_tab();
	}
}

void SteamService::setProfileForGames(bool onConnect) {
	if (!runningGames.empty()) {
		hardwareService.setPanelOverdrive(true);
		openRgbService.setEffect("Gaming", true);
		PerformanceProfile p = PerformanceProfile::Enum::PERFORMANCE;
		performanceService.setPerformanceProfile(p, true, true);

		std::optional<std::string> sched = configuration.getConfiguration().platform.performance.scheduler;
		for (const auto& [key, value] : runningGames) {
			if (value.scheduler.has_value() && value.scheduler != performanceService.getCurrentScheduler()) {
				sched = value.scheduler;
				break;
			}
		}
		performanceService.setScheduler(sched, true);
	} else if (!onConnect) {
		hardwareService.setPanelOverdrive(false);
		openRgbService.restoreAura();
		performanceService.restore();
	}
}

void SteamService::onConnect(bool onBoot) {
	logger.info("Connected to Steam");
	Logger::add_tab();
	if (!onBoot) {
		setProfileForGames(true);
	}
	eventBus.emitGameEvent(runningGames.size());
	Logger::rem_tab();
}

void SteamService::onDisconnect() {
	logger.info("Disconnected from Steam");
	Logger::add_tab();
	if (!runningGames.empty()) {
		performanceService.restoreProfile();
		openRgbService.restoreAura();
		runningGames.clear();
	}
	Logger::rem_tab();
}

const SteamGameConfig SteamService::getConfiguration(const std::string& gid) {
	SteamGameConfig cfg;

	auto games = configuration.getConfiguration().games;
	auto it	   = games.find(gid);

	std::optional<GameEntry> entry = std::nullopt;
	if (it != games.end()) {
		entry = it->second;
	} else {
		for (const auto& [key, val] : configuration.getConfiguration().games) {
			if (val.overlayId == gid) {
				entry = val;
				break;
			}
		}
	}

	if (entry.has_value()) {
		auto gameEntry = entry.value();
		if (gameEntry.env.has_value()) {
			std::istringstream ss(gameEntry.env.value());
			std::string token;
			std::unordered_map<std::string, std::string> env_vars;

			while (ss >> token) {
				auto pos = token.find('=');
				if (pos != std::string::npos) {
					std::string key	  = token.substr(0, pos);
					std::string value = token.substr(pos + 1);
					env_vars[key]	  = value;
				}
			}

			for (const auto& [key, val] : env_vars) {
				cfg.environment[key] = val;
			}
		}
		if (gameEntry.args.has_value()) {
			cfg.parameters = gameEntry.args.value();
		}

		if (gameEntry.proton) {
			cfg.environment["SteamDeck"] = gameEntry.device == ComputerType::Enum::STEAM_DECK ? "1" : "0";
			if (gameEntry.sync != WineSyncOption::Enum::AUTO) {
				cfg.environment["PROTON_USE_NTSYNC"] = gameEntry.sync == WineSyncOption::Enum::NTSYNC ? "1" : "0";
				cfg.environment["PROTON_NO_NTSYNC"]	 = gameEntry.sync == WineSyncOption::Enum::NTSYNC ? "0" : "1";
				cfg.environment["PROTON_NO_FSYNC"]	 = gameEntry.sync == WineSyncOption::Enum::FSYNC ? "0" : "1";
				cfg.environment["PROTON_NO_ESYNC"]	 = gameEntry.sync == WineSyncOption::Enum::ESYNC ? "0" : "1";
			}
		}

		if (gameEntry.gpu.has_value()) {
			auto gpuEnv = hardwareService.getGpuSelectorEnv(gameEntry.gpu.value());
			for (const auto& [key, val] : gpuEnv) {
				cfg.environment[key] = val;
			}
		}

		if (whichSystemdInhibit.has_value()) {
			cfg.wrappers.emplace_back(whichSystemdInhibit.value());
			cfg.wrappers.emplace_back("--who");
			cfg.wrappers.emplace_back("\"" + entry->name + "\"");
		}

		if (whichMangohud.has_value() && gameEntry.metrics_level != MangoHudLevelMeta::Enum::NO_DISPLAY) {
			cfg.environment["MANGOHUD_CONFIG"] = "preset=" + std::to_string(gameEntry.metrics_level.getPresetIndex());
			cfg.environment["MANGOHUD_DLSYM"]  = "1";
			cfg.environment["MANGOHUD"]		   = "1";
			cfg.wrappers.emplace_back(whichMangohud.value());
		}

		if (gameEntry.wrappers.has_value()) {
			auto wraps = StringUtils::split(gameEntry.wrappers.value(), ' ');
			for (auto wrap : wraps) {
				cfg.wrappers.emplace_back(wrap);
			}
		}
	} else {
		logger.error("No configuration entry found for " + gid);
	}

	return cfg;
}

std::string SteamService::encodeAppId(uint32_t appid) {
	return std::to_string((static_cast<uint64_t>(appid) << 32) | 0x02000000ULL);
}

std::optional<std::string> SteamService::getImagePath(uint gid, std::string sufix) {
	const std::vector<std::string> extensions = {"jpg", "png"};
	for (auto ext : extensions) {
		auto path = Constants::LOGOS_DIR + "/" + std::to_string(gid) + sufix + "." + ext;
		if (FileUtils::exists(path)) {
			return path;
		}
	}
	return std::nullopt;
}

std::optional<std::string> SteamService::getIcon(uint gid) {
	return getImagePath(gid, "_logo");
}

std::optional<std::string> SteamService::getBanner(uint gid) {
	return getImagePath(gid, "_hero");
}
