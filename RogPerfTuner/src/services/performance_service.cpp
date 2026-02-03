#include "services/performance_service.hpp"

#include <optional>
#include <string>
#include <thread>

#include "models/cpu_usage.hpp"
#include "models/performance/cpu_governor.hpp"
#include "models/performance/performance_profile.hpp"
#include "models/performance/power_profile.hpp"
#include "utils/configuration_wrapper.hpp"
#include "utils/enum_utils.hpp"
#include "utils/event_bus_wrapper.hpp"
#include "utils/file_utils.hpp"
#include "utils/process_utils.hpp"
#include "utils/string_utils.hpp"
#include "utils/time_utils.hpp"

int8_t PerformanceService::CPU_PRIORITY = -17;
uint8_t PerformanceService::IO_PRIORITY = (CPU_PRIORITY + 20) / 5;
uint8_t PerformanceService::IO_CLASS	= 2;

PerformanceService::PerformanceService() : Loggable("PerformanceService") {
	logger->info("Initializing PerformanceService");
	Logger::add_tab();

	currentProfile	 = configuration.getConfiguration().platform.performance.profile;
	currentScheduler = configuration.getConfiguration().platform.performance.scheduler;

	if (uPowerClient.available()) {
		onBattery		 = uPowerClient.isOnBattery();
		std::string mode = onBattery ? "battery" : "AC";
		logger->info("Laptop on {} mode", mode);
	}

	if (platformClient.available()) {
		platformClient.setChangePlatformProfileOnAc(false);
		platformClient.setChangePlatformProfileOnBattery(false);
		platformClient.setPlatformProfileLinkedEpp(true);
	}

	eventBus.onBattery([this](bool onBat) {
		onBattery = onBat;
		if (runningGames == 0) {
			restoreProfile();
		}
	});

	eventBus.onApplicationStop([this]() {
		PerformanceProfile p = PerformanceProfile::PERFORMANCE;
		this->setPerformanceProfile(p, true, true, false);
	});

	restore();

	Logger::rem_tab();
}

void PerformanceService::renice(const pid_t& pid) {
	logger->info("Renicing process {}", pid);
	Logger::add_tab();
	std::set<pid_t> ion, ren;
	do {
		ion = ProcessUtils::reniceHierarchy(pid, CPU_PRIORITY);
		ren = ProcessUtils::ioniceHierarchy(pid, IO_CLASS, IO_PRIORITY);
	} while (ion != ren);
	Logger::rem_tab();
}

PerformanceProfile PerformanceService::getPerformanceProfile() {
	return currentProfile;
}

void PerformanceService::setActualPerformanceProfile(PerformanceProfile& profile) {
	std::string profileName = toName(profile);
	logger->info("Applying {} profile", profileName);
	Logger::add_tab();
	try {
		setPlatformProfile(profile);
		setBoost(profile);
		setCpuGovernor(profile);
		setPowerProfile(profile);
		setTdps(profile);
		setTgp(profile);
		setFanCurves(profile);

		Logger::rem_tab();
		logger->info("Profile applied succesfully");
		actualProfile = profile;

	} catch (std::exception& e) {
	}
	Logger::rem_tab();
}

void PerformanceService::smartWorker() {
	while (!stopFlag) {
		std::array<double, 5> buffer{};
		size_t index = 0;
		for (size_t j = 0; j < buffer.size(); j++) {
			for (int i = 0; i < 9; i++ && !stopFlag) {
				TimeUtils::sleep(100);
			}
			if (!stopFlag) {
				auto usage	  = CPUUsage::getUseRate(100);
				buffer[index] = usage;
				if (++index == buffer.size()) {
					break;
				}
			}
		}
		if (!stopFlag) {
			double mean = std::accumulate(buffer.begin(), buffer.end(), 0.0) / buffer.size();
			auto next	= actualProfile;
			if (mean > 0.67) {
				next = getNextPerformanceProfile(actualProfile, false);
			} else if (mean < 0.25) {
				next = getPreviousPerformanceProfile(actualProfile, false);
			}
			logger->debug("Mean CPU load: {:.2f}% -> {}", mean * 100, toName(next));
			if (next != actualProfile) {
				setActualPerformanceProfile(next);
			}
		}
	}
}

void PerformanceService::setPerformanceProfile(PerformanceProfile& profile, const bool& temporal, const bool& force, const bool& showToast) {
	std::lock_guard<std::mutex> lock(actionMutex);
	std::string profileName = toName(profile);

	Logger::add_tab();
	if (profile != currentProfile || force) {
		logger->info("Setting {} profile", profileName);

		if (profile != PerformanceProfile::SMART) {
			stopFlag.store(true);
			smartThread->detach();
			smartThread = std::nullopt;
			setActualPerformanceProfile(profile);
		} else {
			auto perf = PerformanceProfile::QUIET;
			setActualPerformanceProfile(perf);
			stopFlag.store(false);
			smartThread = std::thread(&PerformanceService::smartWorker, this);
		}

		if (!temporal) {
			configuration.getConfiguration().platform.performance.profile = profile;
			configuration.saveConfig();
		}

		if (showToast) {
			toaster.showToast(translator.translate("profile.applied",
												   {{"profile", StringUtils::toLowerCase(translator.translate("label.profile." + profileName))}}));
		}

		currentProfile = profile;
	} else {
		logger->info("Profile {} already setted", StringUtils::toLowerCase(profileName));
	}
	Logger::rem_tab();
	eventBus.emitPerformanceProfile(profile);
}

void PerformanceService::setPlatformProfile(const PerformanceProfile& profile) {
	if (platformClient.available()) {
		auto platformProfile = getPlatformProfile(profile);
		logger->info("Platform profile: {}", toName<PlatformProfile>(platformProfile));
		Logger::add_tab();
		try {
			platformClient.setPlatformProfile(platformProfile);
			platformClient.setEnablePptGroup(true);
		} catch (std::exception& e) {
			logger->error("Error while setting platform profile: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void PerformanceService::setFanCurves(const PerformanceProfile& profile) {
	if (asusCtlClient.available()) {
		auto platformProfile = getPlatformProfile(profile);
		logger->info("Fan profile: {}", toName<PlatformProfile>(platformProfile));
		Logger::add_tab();
		try {
			auto it = configuration.getConfiguration().platform.curves.find(toString(profile));

			if (it == configuration.getConfiguration().platform.curves.end()) {
				configuration.getConfiguration().platform.curves[toString(profile)] = {};

				asusCtlClient.setCurvesToDefaults(platformProfile);
				auto data = asusCtlClient.getFanCurveData(platformProfile);
				for (auto& [fan, curve] : data) {
					configuration.getConfiguration().platform.curves[toString(profile)][fan].presets = curve.toData();
					configuration.getConfiguration().platform.curves[toString(profile)][fan].current = curve.toData();
				}

				configuration.saveConfig();
			}

			for (PlatformProfile pp : values<PlatformProfile>()) {
				Logger::add_tab();
				asusCtlClient.setFanCurvesEnabled(pp, false);
				Logger::rem_tab();
			}

			for (const auto& [fan, data] : configuration.getConfiguration().platform.curves[toString(profile)]) {
				logger->info(fan + ": " + StringUtils::replaceAll(data.current, ",", " "));
				Logger::add_tab();
				asusCtlClient.setFanCurveStringData(platformProfile, fan, data.current);
				Logger::rem_tab();
			}

			asusCtlClient.setFanCurvesEnabled(platformProfile, true);
		} catch (std::exception& e) {
			logger->error("Error while setting fan curve: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void PerformanceService::setBoost(const PerformanceProfile&) {
	if (boostControlClient.available()) {
		bool enabled = onBattery ? batteryBoost() : acBoost();
		logger->info("CPU boost: {}", enabled ? "ON" : "OFF");
		Logger::add_tab();
		try {
			boostControlClient.set_boost(enabled);
		} catch (std::exception& e) {
			logger->error("Error while setting CPU boost: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void PerformanceService::setCpuGovernor(const PerformanceProfile& profile) {
	if (cpuPowerClient.available()) {
		CpuGovernor cpuGovernor = onBattery ? batteryGovernor() : acGovernor(profile);
		logger->info("CPU governor: {}", toName(cpuGovernor));
		Logger::add_tab();
		try {
			cpuPowerClient.setGovernor(cpuGovernor);
		} catch (std::exception& e) {
			logger->error("Error while setting CPU governor: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void PerformanceService::setPowerProfile(PerformanceProfile& profile) {
	if (powerProfileClient.available()) {
		PowerProfile powerProfile = getPowerProfile(profile);
		logger->info("Power profile: {}", toName(powerProfile));
		Logger::add_tab();
		try {
			powerProfileClient.setPowerProfile(powerProfile);
		} catch (std::exception& e) {
			logger->error("Error while setting power profile: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void PerformanceService::setTdps(const PerformanceProfile& profile) {
	try {
		if (pl1SpdClient.available()) {
			logger->info("TDP values");
			Logger::add_tab();
			auto pl1 = onBattery ? batteryIntelPl1Spl(profile) : acIntelPl1Spl(profile);
			logger->info("PL1: {}W", pl1);
			pl1SpdClient.setCurrentValue(pl1);
			TimeUtils::sleep(25);

			if (pl2SpptClient.available()) {
				auto pl2 = onBattery ? batteryIntelPl2Sppt(profile) : acIntelPl2Sppt(profile);
				logger->info("PL2: {}W", pl2);
				pl2SpptClient.setCurrentValue(pl2);
				TimeUtils::sleep(25);
			}

			if (pl3FpptClient.available()) {
				auto pl3 = onBattery ? batteryIntelPl3Fppt(profile) : acIntelPl3Fppt(profile);
				logger->info("PL3: {}W", pl3);
				pl3FpptClient.setCurrentValue(pl3);
				TimeUtils::sleep(25);
			}

			Logger::rem_tab();
		}
	} catch (std::exception& e) {
		logger->error("Error setting CPU TDPs: {}", e.what());
	}
}

void PerformanceService::setTgp(const PerformanceProfile& profile) {
	if (nvTempClient.available() || nvBoostClient.available()) {
		logger->info("Nvidia GPU");
		Logger::add_tab();

		if (nvBoostClient.available()) {
			try {
				auto nvb = onBattery ? batteryNvBoost(profile) : acNvBoost(profile);
				logger->info("Dynamic Boost: {}W", nvb);
				nvBoostClient.setCurrentValue(nvb);
				TimeUtils::sleep(25);
			} catch (std::exception& e) {
				logger->error("Error setting Nvidia Boost: {}", e.what());
			}
		}

		if (nvTempClient.available()) {
			try {
				auto nvt = onBattery ? batteryNvTemp(profile) : acNvTemp();
				logger->info("Throttle temp: {}ºC", nvt);
				nvTempClient.setCurrentValue(nvt);
			} catch (std::exception& e) {
				logger->error("Error setting Nvidia TGP: {}", e.what());
			}
		}

		Logger::rem_tab();
	}
}

void PerformanceService::restore() {
	restoreProfile();
	restoreScheduler();
}

void PerformanceService::restoreProfile() {
	if (onBattery) {
		PerformanceProfile p = PerformanceProfile::QUIET;
		setPerformanceProfile(p, true, true);
	} else {
		setPerformanceProfile(configuration.getConfiguration().platform.performance.profile, false, true);
	}
}

void PerformanceService::restoreScheduler() {
	setScheduler(configuration.getConfiguration().platform.performance.scheduler);
}

PerformanceProfile PerformanceService::nextPerformanceProfile() {
	auto nextProfile = getNextPerformanceProfile(currentProfile);
	setPerformanceProfile(nextProfile);
	return nextProfile;
}

int PerformanceService::acIntelPl1Spl(PerformanceProfile profile) {
	auto& client = pl1SpdClient;

	if (profile == PerformanceProfile::PERFORMANCE) {
		return client.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return client.getMaxValue() * 0.75;
	}
	if (profile == PerformanceProfile::QUIET) {
		return client.getMaxValue() * 0.55;
	}

	return client.getCurrentValue();
}

int PerformanceService::batteryIntelPl1Spl(PerformanceProfile profile) {
	auto& client = pl1SpdClient;
	return acTdpToBatteryTdp(acIntelPl1Spl(profile), client.getMinValue());
}

int PerformanceService::acIntelPl2Sppt(PerformanceProfile profile) {
	auto& client = pl2SpptClient;

	if (!acBoost()) {
		return acIntelPl1Spl(profile);
	}

	if (profile == PerformanceProfile::PERFORMANCE) {
		return client.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return client.getMaxValue() * 0.85;
	}
	if (profile == PerformanceProfile::QUIET) {
		return client.getMaxValue() * 0.7;
	}

	return client.getCurrentValue();
}

int PerformanceService::batteryIntelPl2Sppt(PerformanceProfile profile) {
	auto& client = pl2SpptClient;
	return acTdpToBatteryTdp(acIntelPl2Sppt(profile), client.getMinValue());
}

int PerformanceService::acIntelPl3Fppt(PerformanceProfile profile) {
	auto& client = pl2SpptClient;

	if (!acBoost()) {
		return acIntelPl1Spl(profile);
	}

	if (profile == PerformanceProfile::PERFORMANCE) {
		return client.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return client.getMaxValue() * 0.9;
	}
	if (profile == PerformanceProfile::QUIET) {
		return client.getMaxValue() * 0.8;
	}

	return client.getCurrentValue();
}

int PerformanceService::batteryIntelPl3Fppt(PerformanceProfile profile) {
	auto& client = pl3FpptClient;
	return acTdpToBatteryTdp(acIntelPl2Sppt(profile), client.getMinValue());
}

int PerformanceService::acNvBoost(PerformanceProfile profile) {
	auto& client = nvBoostClient;

	if (profile == PerformanceProfile::PERFORMANCE) {
		return client.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return (client.getMaxValue() + client.getMinValue()) / 2;
	}
	if (profile == PerformanceProfile::QUIET) {
		return client.getMinValue();
	}

	return client.getCurrentValue();
}

int PerformanceService::batteryNvBoost(PerformanceProfile profile) {
	auto& client = nvBoostClient;
	return acTdpToBatteryTdp(acNvBoost(profile), client.getMinValue());
}

int PerformanceService::acNvTemp() {
	auto& client = nvTempClient;
	return client.getMaxValue();
}

int PerformanceService::batteryNvTemp(PerformanceProfile profile) {
	auto& client = nvTempClient;

	if (profile == PerformanceProfile::PERFORMANCE) {
		return client.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return (client.getMaxValue() + client.getMinValue()) / 2;
	}
	if (profile == PerformanceProfile::QUIET) {
		return client.getMinValue();
	}

	return client.getCurrentValue();
}

bool PerformanceService::acBoost() {
	return true;
}
bool PerformanceService::batteryBoost() {
	return false;
}

CpuGovernor PerformanceService::acGovernor(PerformanceProfile profile) {
	if (profile == PerformanceProfile::PERFORMANCE) {
		return CpuGovernor::PERFORMANCE;
	}
	return CpuGovernor::POWERSAVE;
}

CpuGovernor PerformanceService::batteryGovernor() {
	return CpuGovernor::POWERSAVE;
}

int PerformanceService::acTdpToBatteryTdp(int tdp, int minTdp) {
	return std::max(minTdp, static_cast<int>(std::round(tdp * 0.6)));
}

std::vector<std::string> PerformanceService::getAvailableSchedulers() {
	if (!scxCtlClient.available()) {
		return {};
	}

	return scxCtlClient.getAvailable();
}

std::optional<std::string> PerformanceService::getCurrentScheduler() {
	if (!scxCtlClient.available()) {
		return std::nullopt;
	}

	return currentScheduler;
}

void PerformanceService::setScheduler(std::optional<std::string> scheduler, bool temporal) {
	if (!scxCtlClient.available()) {
		return;
	}

	if (scheduler.has_value()) {
		scxCtlClient.start(scheduler.value());
	} else {
		scxCtlClient.stop();
	}

	currentScheduler = scheduler;

	if (!temporal) {
		configuration.getConfiguration().platform.performance.scheduler = scheduler;
		configuration.saveConfig();
	}

	eventBus.emitScheduler(scheduler);
}

std::vector<std::string> PerformanceService::getFans() {
	std::vector<std::string> res;

	for (const auto& [key, val] : configuration.getConfiguration().platform.curves[toString(currentProfile)]) {
		res.emplace_back(key);
	}

	return res;
}

FanCurveData PerformanceService::getFanCurve(std::string fan, std::string profile) {
	return FanCurveData::fromData(configuration.getConfiguration().platform.curves[profile][fan].current);
}

FanCurveData PerformanceService::getDefaultFanCurve(std::string fan, std::string profile) {
	return FanCurveData::fromData(configuration.getConfiguration().platform.curves[profile][fan].presets);
}

void PerformanceService::saveFanCurves(std::string profile, std::unordered_map<std::string, FanCurveData> curves) {
	logger->info("Saving curves for {}", profile);
	Logger::add_tab();

	auto pp = getPlatformProfile(fromString<PerformanceProfile>(profile));

	asusCtlClient.setFanCurvesEnabled(pp, false);
	for (const auto& [fan, curve] : curves) {
		configuration.getConfiguration().platform.curves[profile][fan].current = curve.toData();
	}
	setFanCurves(fromString<PerformanceProfile>(profile));
	asusCtlClient.setFanCurvesEnabled(pp, true);

	configuration.saveConfig();

	Logger::rem_tab();
	logger->info("Fan curved updated succesfully");
}

std::string PerformanceService::getDefaultSchedulerName() {
	const std::string borePath = "/proc/sys/kernel/sched_bore";
	if (FileUtils::exists(borePath) && StringUtils::trim(FileUtils::readFileContent(borePath)) == "1") {
		return "Bore";
	}
	return "Eevdf";
}