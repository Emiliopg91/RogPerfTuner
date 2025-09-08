#include "../../include/services/steam_service.hpp"

#include <iostream>
#include <set>
#include <sstream>

#include "../../include/configuration/configuration.hpp"
#include "../../include/models/hardware/gpu_brand.hpp"
#include "../../include/models/others/semantic_version.hpp"
#include "../../include/models/steam/steam_game_details.hpp"
#include "../../include/services/hardware_service.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/services/profile_service.hpp"
#include "RccCommons.hpp"

bool SteamService::metricsEnabled() {
	auto mangohud_which = shell.which("mangohud");
	return mangohud_which.has_value();
}

bool SteamService::isRunning(const unsigned int& appid) const {
	for (const auto& [key, val] : runningGames)
		if (key == appid)
			return true;
	return false;
}

const std::map<unsigned int, std::string>& SteamService::getRunningGames() const {
	return runningGames;
}

const std::map<std::string, GameEntry>& SteamService::getGames() {
	return configuration.getConfiguration().games;
}

SteamService::SteamService() {
	logger.info("Initializing SteamClient");
	logger.add_tab();

	SteamClient::getInstance();
	std::this_thread::sleep_for(std::chrono::milliseconds(25));
	if (SteamClient::getInstance().connected()) {
		onConnect(true);
	}

	installRccDC();

	SteamClient::getInstance().onConnect([this]() { onConnect(); });

	SteamClient::getInstance().onDisconnect([this]() { onDisconnect(); });

	SteamClient::getInstance().onGameLaunch([this](std::vector<std::any> data) {
		auto id	  = static_cast<uint32_t>(std::any_cast<int>(data[0]));
		auto name = std::any_cast<std::string>(data[1]);
		auto pid  = std::any_cast<int>(data[2]);

		onGameLaunch(id, name, pid);
	});

	SteamClient::getInstance().onGameStop([this](std::vector<std::any> data) {
		auto id	  = static_cast<uint32_t>(std::any_cast<int>(data[0]));
		auto name = std::any_cast<std::string>(data[1]);

		onGameStop(id, name);
	});

	logger.rem_tab();
}

void getPidsOfHierarchy(const pid_t parentId, std::set<pid_t>& pids) {
	for (auto& entry : std::filesystem::directory_iterator("/proc")) {
		std::string filename = entry.path().filename();
		if (entry.is_directory() && std::all_of(filename.begin(), filename.end(), ::isdigit)) {
			auto pid = std::stoi(filename);
			if (pids.find(pid) == pids.end()) {
				std::ifstream statFile("/proc/" + filename + "/stat");
				if (statFile.is_open()) {
					int pid_read, ppid;
					std::string comm, state;
					statFile >> pid_read >> comm >> state >> ppid;

					if (ppid == parentId) {
						pids.insert(pid);
						getPidsOfHierarchy(pid, pids);
					}
				}
			}
		}
	}
}

std::map<std::string, std::string> getProcessEnvironMap(pid_t pid) {
	std::map<std::string, std::string> env_map;
	std::string path = "/proc/" + std::to_string(pid) + "/environ";
	std::ifstream file(path, std::ios::in | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("No se pudo abrir " + path);
	}

	std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	size_t start = 0;
	while (start < buffer.size()) {
		size_t end = buffer.find('\0', start);
		if (end == std::string::npos)
			break;
		std::string entry = buffer.substr(start, end - start);
		size_t eq_pos	  = entry.find('=');
		if (eq_pos != std::string::npos) {
			std::string key	  = entry.substr(0, eq_pos);
			std::string value = entry.substr(eq_pos + 1);
			env_map[key]	  = value;
		}
		start = end + 1;
	}

	return env_map;
}

void SteamService::onFirstGameRun(unsigned int gid, std::string name, std::map<std::string, std::string> environment) {
	logger.info("Configuring game");
	logger.add_tab();

	std::optional<std::string> gpu = std::nullopt;
	if (hardwareService.getGpus().size() > 0) {
		auto items = GpuBrand::getAll();
		auto gpus  = hardwareService.getGpus();
		for (GpuBrand g : GpuBrand::getAll()) {
			if (gpus.find(g.toString()) != gpus.end()) {
				gpu = g.toString();
				break;
			}
		}
	}

	auto proton = environment.find("STEAM_COMPAT_PROTON") != environment.end();

	SteamGameDetails details = SteamClient::getInstance().getAppsDetails({gid})[0];
	auto launch_opts		 = details.launch_opts;

	std::string env, args;

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

	auto overlayId = environment.find("SteamOverlayGameId")->second;

	GameEntry entry{args, env, gpu, MangoHudLevel::Enum::NO_DISPLAY, name, overlayId, proton, false, WineSyncOption::Enum::AUTO};
	configuration.getConfiguration().games[std::to_string(gid)] = entry;
	configuration.saveConfig();

	SteamClient::getInstance().setLaunchOptions(gid, WRAPPER_PATH + " %command%");
	logger.info("Configuration finished");

	logger.info("Relaunching game with SteamOverlayId " + overlayId + "...");
	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	shell.run_command("steam steam://rungameid/" + overlayId);

	logger.rem_tab();
	logger.rem_tab();
}

bool SteamService::checkIfRequiredInstallation() {
	std::ifstream fileRunning(Constants::RCCDC_ASSET_PATH + "/package.json");

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
	} catch (std::exception e) {
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
	logger.info("Launched '" + name + "' (" + std::to_string(gid) + ") with PID " + std::to_string(pid));
	logger.add_tab();
	if (configuration.getConfiguration().games.find(std::to_string(gid)) == configuration.getConfiguration().games.end()) {
		logger.info("Game not configured");
		logger.add_tab();

		logger.info("Getting process environment...");
		logger.add_tab();
		auto env = getProcessEnvironMap(pid);
		logger.rem_tab();

		logger.info("Stopping process...");
		logger.add_tab();
		std::set<pid_t> pids;
		pids.insert(pid);
		getPidsOfHierarchy(pid, pids);

		std::ostringstream oss;
		oss << "kill -9";
		for (pid_t pid : pids) {
			oss << " " << pid;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		shell.run_elevated_command(oss.str(), false);
		logger.rem_tab();

		std::thread([this, gid, name, env]() { onFirstGameRun(gid, name, env); }).detach();
		;
	} else if (runningGames.find(gid) == runningGames.end()) {
		runningGames[gid] = name;
		setProfileForGames();

		eventBus.emit_event(Events::STEAM_SERVICE_GAME_EVENT, {runningGames.size()});
	}
	logger.rem_tab();
}

void SteamService::onGameStop(unsigned int gid, std::string name) {
	if (runningGames.find(gid) != runningGames.end()) {
		logger.info("Stopped '" + name + "' (" + std::to_string(gid) + ")");
		runningGames.erase(gid);
		{
			logger.add_tab();
			setProfileForGames();
			logger.rem_tab();

			eventBus.emit_event(Events::STEAM_SERVICE_GAME_EVENT, {runningGames.size()});
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
	logger.add_tab();
	if (!onBoot) {
		setProfileForGames(true);
	}
	eventBus.emit_event(Events::STEAM_SERVICE_GAME_EVENT, {runningGames.size()});
	logger.rem_tab();
}

void SteamService::onDisconnect() {
	logger.info("Disconnected from Steam");
	logger.add_tab();
	if (!runningGames.empty()) {
		profileService.restoreProfile();
		openRgbService.restoreAura();
		runningGames.clear();
	}
	logger.rem_tab();
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
			std::map<std::string, std::string> env_vars;

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
	} else {
		logger.error("No configuration entry found for " + gid);
	}

	return cfg;
}

const std::optional<GpuBrand> SteamService::getPreferedGpu(const unsigned int& gid) {
	return configuration.getConfiguration().games[std::to_string(gid)].gpu;
}

void SteamService::setPreferedGpu(const unsigned int& gid, const std::optional<GpuBrand>& gpu) {
	if (gpu.has_value())
		configuration.getConfiguration().games[std::to_string(gid)].gpu = gpu.value().toString();
	else
		configuration.getConfiguration().games[std::to_string(gid)].gpu = std::nullopt;

	configuration.saveConfig();
}

bool SteamService::isSteamDeck(const unsigned int& gid) {
	return configuration.getConfiguration().games[std::to_string(gid)].steamdeck;
}

void SteamService::setSteamDeck(const unsigned int& gid, const bool& value) {
	configuration.getConfiguration().games[std::to_string(gid)].steamdeck = value;
	configuration.saveConfig();
}

const MangoHudLevel SteamService::getMetricsLevel(const unsigned int& gid) {
	return configuration.getConfiguration().games[std::to_string(gid)].metrics_level;
}

void SteamService::setMetricsLevel(const unsigned int& gid, const MangoHudLevel& level) {
	configuration.getConfiguration().games[std::to_string(gid)].metrics_level = level;
	configuration.saveConfig();
}

const WineSyncOption SteamService::getWineSync(const unsigned int& gid) {
	return configuration.getConfiguration().games[std::to_string(gid)].sync;
}

void SteamService::setWineSync(const unsigned int& gid, const WineSyncOption& level) {
	configuration.getConfiguration().games[std::to_string(gid)].sync = level;
	configuration.saveConfig();
}

bool SteamService::isProton(const unsigned int& gid) {
	return configuration.getConfiguration().games[std::to_string(gid)].proton;
}

const std::string SteamService::getEnvironment(const unsigned int& gid) {
	return configuration.getConfiguration().games[std::to_string(gid)].env.value_or("");
}

void SteamService::setEnvironment(const unsigned int& gid, const std::string& env) {
	configuration.getConfiguration().games[std::to_string(gid)].env = env;
	configuration.saveConfig();
}

const std::string SteamService::getParameters(const unsigned int& gid) {
	return configuration.getConfiguration().games[std::to_string(gid)].args.value_or("");
}

void SteamService::setParameters(const unsigned int& gid, const std::string& args) {
	configuration.getConfiguration().games[std::to_string(gid)].args = args;
	configuration.saveConfig();
}