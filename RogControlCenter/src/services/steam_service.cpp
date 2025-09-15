#include "../../include/services/steam_service.hpp"

#include <chrono>
#include <csignal>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>

#include "../../include/configuration/configuration.hpp"
#include "../../include/events/event_bus.hpp"
#include "../../include/gui/game_config_dialog.hpp"
#include "../../include/models/others/semantic_version.hpp"
#include "../../include/models/steam/steam_game_details.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/services/profile_service.hpp"

bool SteamService::metricsEnabled() {
	auto mangohud_which = shell.which("mangohud");
	return mangohud_which.has_value();
}

bool SteamService::isRunning(const unsigned int& appid) const {
	for (const auto& [key, val] : runningGames) {
		if (key == appid) {
			return true;
		}
	}
	return false;
}

const std::unordered_map<unsigned int, std::string>& SteamService::getRunningGames() const {
	return runningGames;
}

const std::unordered_map<std::string, GameEntry>& SteamService::getGames() {
	return configuration.getConfiguration().games;
}

SteamService::SteamService() {
	logger.info("Initializing SteamService");
	Logger::add_tab();

	std::this_thread::sleep_for(std::chrono::milliseconds(25));
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
		auto id	  = static_cast<uint32_t>(std::any_cast<int>(data[0]));
		auto name = std::any_cast<std::string>(data[1]);
		auto pid  = std::any_cast<int>(data[2]);

		onGameLaunch(id, name, pid);
	});

	steamClient.onGameStop([this](CallbackParam data) {
		auto id	  = static_cast<uint32_t>(std::any_cast<int>(data[0]));
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
				if (token != Constants::WRAPPER_PATH) {
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
					MangoHudLevel::Enum::NO_DISPLAY,
					name,
					details.is_shortcut ? std::optional<std::string>{encodeAppId(gid)} : std::nullopt,
					!details.compat_tool.empty(),
					false,
					WineSyncOption::Enum::AUTO,
					wrappers};
	configuration.getConfiguration().games[std::to_string(gid)] = entry;
	configuration.saveConfig();

	steamClient.setLaunchOptions(gid, Constants::WRAPPER_PATH + " %command%");

	QMetaObject::invokeMethod(
		qApp,
		[this, gid]() {
			GameConfigDialog dialog(gid, true);
			dialog.showDialog();
		},
		Qt::QueuedConnection);

	Logger::rem_tab();
	Logger::rem_tab();
}

void SteamService::saveGameConfig(uint gid, const GameEntry& entry) {
	logger.info("Saving configuration for '{}' ({})", entry.name, gid);
	Logger::add_tab();

	configuration.getConfiguration().games[std::to_string(gid)] = entry;
	configuration.saveConfig();

	Logger::rem_tab();
}

void SteamService::launchGame(const std::string& id) {
	logger.info("Launching game with id {}...", id);
	Logger::add_tab();

	shell.run_command("steam steam://rungameid/" + id);

	Logger::rem_tab();
}

bool SteamService::checkIfRequiredInstallation() {
	std::ifstream fileRunning(Constants::RCCDC_PACKAGE_FILE);

	nlohmann::json jR;
	fileRunning >> jR;

	SemanticVersion vA = SemanticVersion::parse(Constants::PLUGIN_VERSION);
	SemanticVersion vR = SemanticVersion::parse(jR["version"]);

	return vA > vR;
}

void SteamService::installRccDC() {
	try {
		if (FileUtils::exists(Constants::DECKY_SERVICE_PATH)) {
			if (!FileUtils::exists(Constants::RCCDC_PATH)) {
				logger.info("Installing plugin for first time");
				copyPlugin();
			} else {
				if (checkIfRequiredInstallation()) {
					logger.info("Updating Decky plugin");
					copyPlugin();
				} else {
					logger.info("Plugin up to date");
				}
			}
			rccdcEnabled = true;
		} else {
			logger.warn("No Decky installation found, skipping plugin installation");
			rccdcEnabled = false;
		}
	} catch (std::exception& e) {
		logger.error("Error while installing RCCDeckyCompanion plugin: {}", e.what());
	}
}

void SteamService::copyPlugin() {
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
}

void SteamService::onGameLaunch(unsigned int gid, std::string name, int pid) {
	logger.info("Launched '{}' ({}) with PID {}", name, gid, pid);
	Logger::add_tab();
	if (configuration.getConfiguration().games.find(std::to_string(gid)) == configuration.getConfiguration().games.end()) {
		logger.info("Game not configured");
		Logger::add_tab();

		logger.info("Stopping process...");

		const std::string stopCmd =
			"pstree -p " + std::to_string(pid) + " | grep -o '([0-9]\\+)' | grep -o '[0-9]\\+' | tee >(xargs -r kill -19 2>/dev/null) | wc -l";
		const std::string killCmd =
			"pstree -p " + std::to_string(pid) + " | grep -o '([0-9]\\+)' | grep -o '[0-9]\\+' | tee >(xargs -r kill -9 2>/dev/null) | wc -l";

		uint signaled	 = 0;
		uint newSignaled = 0;
		do {
			signaled	= newSignaled;
			newSignaled = static_cast<uint>(std::stoul(StringUtils::trim(shell.run_elevated_command(stopCmd).stdout_str)));

			logger.debug("Stopped {} processes, before {}", newSignaled, signaled);

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		} while (signaled < newSignaled);
		logger.debug("Killed {} processes", static_cast<uint>(std::stoul(StringUtils::trim(shell.run_elevated_command(killCmd).stdout_str))));

		Logger::rem_tab();

		std::thread([this, gid, name]() {
			onFirstGameRun(gid, name);
		}).detach();
	} else if (runningGames.find(gid) == runningGames.end()) {
		runningGames[gid] = name;
		setProfileForGames();

		eventBus.emitGameEvent(runningGames.size());
	}
	Logger::rem_tab();
}

void SteamService::onGameStop(unsigned int gid, std::string name) {
	if (runningGames.find(gid) != runningGames.end()) {
		logger.info("Stopped '{}' ({})", name, gid);
		runningGames.erase(gid);
		{
			Logger::add_tab();
			setProfileForGames();

			eventBus.emitGameEvent(runningGames.size());
			Logger::rem_tab();
		}
	}
}

void SteamService::setProfileForGames(bool onConnect) {
	if (!runningGames.empty()) {
		hardwareService.setPanelOverdrive(true);
		openRgbService.setEffect("Gaming", true);
		profileService.setPerformanceProfile(PerformanceProfile::Enum::PERFORMANCE, true, true);
	} else if (!onConnect) {
		hardwareService.setPanelOverdrive(false);
		openRgbService.restoreAura();
		profileService.restoreProfile();
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
		profileService.restoreProfile();
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
				logger.info("Added env {}={}", key, val);
			}
		}
		if (gameEntry.args.has_value()) {
			cfg.parameters = gameEntry.args.value();
		}

		cfg.environment["SteamDeck"] = gameEntry.steamdeck ? "1" : "0";
		if (gameEntry.proton) {
			cfg.environment["PROTON_USE_NTSYNC"] = gameEntry.sync == WineSyncOption::Enum::NTSYNC ? "1" : "0";
			cfg.environment["PROTON_NO_NTSYNC"]	 = gameEntry.sync == WineSyncOption::Enum::NTSYNC ? "0" : "1";
			cfg.environment["PROTON_NO_FSYNC"]	 = gameEntry.sync == WineSyncOption::Enum::FSYNC ? "0" : "1";
			cfg.environment["PROTON_NO_ESYNC"]	 = gameEntry.sync == WineSyncOption::Enum::ESYNC ? "0" : "1";
		}

		if (gameEntry.gpu.has_value()) {
			auto gpuEnv = hardwareService.getGpuSelectorEnv(gameEntry.gpu.value());
			for (const auto& [key, val] : gpuEnv) {
				cfg.environment[key] = val;
			}
		}

		auto mangohud_which = shell.which("mangohud");
		if (mangohud_which.has_value()) {
			cfg.environment["MANGOHUD_CONFIG"] = "preset=" + std::to_string(gameEntry.metrics_level.toInt());
			cfg.environment["MANGOHUD_DLSYM"]  = "1";
			cfg.environment["MANGOHUD"]		   = "1";
			cfg.wrappers.emplace_back(mangohud_which.value());
		}

		if (gameEntry.wrappers.has_value()) {
			auto wraps = StringUtils::split(gameEntry.wrappers.value(), ' ');
			for (auto wrap : wraps) {
				cfg.wrappers.emplace_back(wrap);
			}
		}
	} else {
		logger.error("No configuration entry found for {}", gid);
	}

	return cfg;
}

std::string SteamService::encodeAppId(uint32_t appid) {
	return std::to_string((static_cast<uint64_t>(appid) << 32) | 0x02000000ULL);
}