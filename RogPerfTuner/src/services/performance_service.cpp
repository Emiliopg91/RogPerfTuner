#include "services/performance_service.hpp"

#include <optional>
#include <string>
#include <thread>

#include "framework/logger/logger.hpp"
#include "framework/models/cpu_usage.hpp"
#include "framework/utils/enum_utils.hpp"
#include "framework/utils/process_utils.hpp"
#include "framework/utils/string_utils.hpp"
#include "framework/utils/time_utils.hpp"
#include "models/performance/performance_profile.hpp"
#include "models/performance/power_profile.hpp"
#include "utils/configuration_wrapper.hpp"
#include "utils/event_bus_wrapper.hpp"

PerformanceService::PerformanceService() : Loggable("PerformanceService") {
	logger->info("Initializing PerformanceService");
	Logger::add_tab();

	currentProfile	 = configuration.getConfiguration().platform.performance.profile;
	currentScheduler = configuration.getConfiguration().platform.performance.scheduler.value_or("");

#ifdef BAT_STATUS
	onBattery		 = batteryStatusClient.isOnBattery();
	std::string mode = onBattery ? "battery" : "AC";
	logger->info("Laptop on {} mode", mode);
#endif

	availableSchedulers = {"EEVDF"};
	if (schedBoreClient.available()) {
		availableSchedulers.emplace_back("BORE");
	}
	if (scxCtlClient.available()) {
		for (auto& sched : scxCtlClient.getAvailable()) {
			availableSchedulers.emplace_back(StringUtils::capitalize(sched));
		}
	}

	defaultScheduler = "EEVDF";
	if (schedBoreClient.available() && StringUtils::trim(schedBoreClient.read()) == "1") {
		defaultScheduler = "BORE";
	}
	if (currentScheduler.empty() ||
		std::find(availableSchedulers.begin(), availableSchedulers.end(), currentScheduler) == availableSchedulers.end()) {
		currentScheduler												= defaultScheduler;
		configuration.getConfiguration().platform.performance.scheduler = currentScheduler;
		configuration.saveConfig();
	}
	logger->info("Available schedulers:");
	Logger::add_tab();
	logger->info(StringUtils::join(availableSchedulers, ", "));
	Logger::rem_tab();

	if (ssdSchedulerClient.available()) {
		logger->info("Available SSD schedulers:");
		Logger::add_tab();
		logger->info(StringUtils::join(getAvailableSsdSchedulers(), ", "));
		Logger::rem_tab();
	}

	platformClient.setChangePlatformProfileOnAc(false);
	platformClient.setChangePlatformProfileOnBattery(false);
	platformClient.setPlatformProfileLinkedEpp(true);

	eventBus.onBattery([this](bool onBat) {
		onBattery = onBat;
		if (runningGames == 0) {
			restore();
		}
	});

	eventBus.onApplicationShutdown([this]() {
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
	std::lock_guard<std::mutex> lock(actProfMutex);

	std::string profileName = toName(profile);
	logger->info("Applying {} profile", profileName);
	auto t0 = TimeUtils::now();
	Logger::add_tab();
	try {
		setPlatformProfile(profile);
#ifdef BOOST_CONTROL
		setBoost(profile);
#endif
#ifdef SCALING_GOVERNOR
		setCpuGovernor(profile);
#endif
#ifdef ACPI_PROFILE
		setPowerProfile(profile);
#endif
#ifdef PPT_PL1_SPL
		setTdps(profile);
#endif
#if defined(NV_THERMAL) || defined(NV_BOOST)
		setNvidiaProfile(profile);
#endif
#ifdef FAN_CONTROL
		setFanCurves(profile);
#endif
		auto t1 = TimeUtils::now();
		logger->info("Profile applied after {} seconds", TimeUtils::format_seconds(TimeUtils::getTimeDiff(t0, t1)));
		actualProfile = profile;

	} catch (std::exception& e) {
	}
	Logger::rem_tab();
}

PerformanceProfile PerformanceService::getNextSmart(size_t samples, int level) {
	auto next = actualProfile;

	std::vector<double> buffer(samples, 0.0);
	size_t index = 0;
	for (size_t j = 0; j < samples; j++) {
		if (!stopFlag) {
			auto usage	  = std::round(CPUUsage::getUseRate(1000) * 100);
			buffer[index] = usage;
			if (++index == samples) {
				break;
			}
		}
	}

	if (!stopFlag) {
		auto avg = std::round(std::accumulate(buffer.begin(), buffer.end(), 0.0) / samples);
		logger->debug("Average CPU usage: {}%", avg);
		if (avg > 67) {
			next = getNextPerformanceProfile(actualProfile, false, false);
			if (next != actualProfile) {
				logger->info("Ramp up to {}", toName(next));
			}

		} else if (avg < 25) {
			next = getPreviousPerformanceProfile(actualProfile, false);
			if (next != actualProfile) {
				if (level != 0) {
					logger->debug("Waiting for {} more checks before ramp down", level);
					next = getNextSmart(samples, level - 1);
				} else {
					logger->info("Ramp down to {}", toName(next));
				}
			}
		}
	}

	return next;
}

void PerformanceService::smartWorker() {
	while (!stopFlag) {
		if (!stopFlag) {
			auto next = getNextSmart();
			if (next != actualProfile) {
				Logger::add_tab();
				setActualPerformanceProfile(next);
				Logger::rem_tab();
			}
		}
	}
}

void PerformanceService::setPerformanceProfile(PerformanceProfile& profile, bool temporal, bool force, bool showToast) {
	std::lock_guard<std::mutex> lock(perProfMutex);
	std::string profileName = toName(profile);

	if (profile != currentProfile || force) {
		logger->info("Setting {} profile", profileName);

		Logger::add_tab();
		if (profile != PerformanceProfile::SMART) {
			stopFlag.store(true);
			if (smartThread.has_value()) {
				if (smartThread->joinable()) {
					logger->info("Waiting for {} worker to stop", toName(PerformanceProfile::SMART));
					smartThread->join();
				}
				smartThread = std::nullopt;
			}
			setActualPerformanceProfile(profile);
		} else {
			logger->info("Starting {} worker", toName(PerformanceProfile::SMART));
			auto perf = PerformanceProfile::PERFORMANCE;
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
		Logger::rem_tab();
	} else {
		logger->info("Profile {} already setted", StringUtils::toLowerCase(profileName));
	}
	eventBus.emitPerformanceProfile(profile);
}

void PerformanceService::setPlatformProfile(const PerformanceProfile& profile) {
	if (!onBattery) {
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
	} else {
		logger->info("Platform profile: Not available on battery");
	}
}

#ifdef BOOST_CONTROL
void PerformanceService::setBoost(const PerformanceProfile&) {
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
bool PerformanceService::acBoost() {
	return true;
}
bool PerformanceService::batteryBoost() {
	return false;
}
#endif

#ifdef SCALING_GOVERNOR
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
#endif

#ifdef ACPI_PROFILE
void PerformanceService::setPowerProfile(PerformanceProfile& profile) {
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
#endif

#ifdef PPT_PL1_SPL
void PerformanceService::setTdps(const PerformanceProfile& profile) {
	try {
		logger->info("TDP values");
		Logger::add_tab();
		auto pl1 = pl1Spl(profile);
		logger->info("PL1: {}W", pl1);
		pl1SpdClient.setCurrentValue(pl1);
		TimeUtils::sleep(25);

#ifdef PPT_PL2_SPPT
		auto pl2 = pl2Sppt(profile);
		logger->info("PL2: {}W", pl2);
		pl2SpptClient.setCurrentValue(pl2);
		TimeUtils::sleep(25);

#ifdef PPT_PL3_FPPT
		auto pl3 = pl3Fppt(profile);
		logger->info("PL3: {}W", pl3);
		pl3FpptClient.setCurrentValue(pl3);
		TimeUtils::sleep(25);
#endif

#endif
		Logger::rem_tab();
	} catch (std::exception& e) {
		logger->error("Error setting CPU TDPs: {}", e.what());
	}
}
#endif

#if defined(NV_THERMAL) || defined(NV_BOOST)
void PerformanceService::setNvidiaProfile(const PerformanceProfile& profile) {
	logger->info("Nvidia GPU");
	Logger::add_tab();

#ifdef NV_BOOST
	if (!onBattery || nvBoostClient.getMinValue() != nvBoostClient.getMaxValue()) {
		try {
			auto nvb = nvBoost(profile);
			logger->info("Dynamic Boost: {}W", nvb);
			nvBoostClient.setCurrentValue(nvb);
			TimeUtils::sleep(25);
		} catch (std::exception& e) {
			logger->error("Error setting Nvidia Boost: {}", e.what());
		}
	} else {
		logger->info("Dynamic Boost: Not available on battery");
	}
#endif
#ifdef NV_THERMAL
	try {
		auto nvt = onBattery ? batteryNvTemp(profile) : acNvTemp();
		logger->info("Throttle temp: {}ºC", nvt);
		nvTempClient.setCurrentValue(nvt);
	} catch (std::exception& e) {
		logger->error("Error setting Nvidia throttle temp: {}", e.what());
	}
#endif

	Logger::rem_tab();
}
#endif

void PerformanceService::restore() {
	if (onBattery) {
		PerformanceProfile p = PerformanceProfile::QUIET;
		setPerformanceProfile(p, true, true);
	} else {
		setPerformanceProfile(configuration.getConfiguration().platform.performance.profile, false, true);
	}

	setScheduler(configuration.getConfiguration().platform.performance.scheduler.value_or(currentScheduler));
	setSsdScheduler(configuration.getConfiguration().platform.performance.ssdScheduler);
}

PerformanceProfile PerformanceService::nextPerformanceProfile() {
	auto nextProfile = getNextPerformanceProfile(currentProfile);
	setPerformanceProfile(nextProfile);
	return nextProfile;
}

#ifdef PPT_PL1_SPL
int PerformanceService::pl1Spl(PerformanceProfile profile) {
	if (profile == PerformanceProfile::PERFORMANCE) {
		return pl1SpdClient.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return pl1SpdClient.getMaxValue() * 0.8;
	}
	if (profile == PerformanceProfile::QUIET) {
		return pl1SpdClient.getMaxValue() * 0.6;
	}

	return pl1SpdClient.getCurrentValue();
}
#endif

#ifdef PPT_PL2_SPPT
int PerformanceService::pl2Sppt(PerformanceProfile profile) {
	if (!acBoost()) {
		return pl1Spl(profile);
	}

	if (profile == PerformanceProfile::PERFORMANCE) {
		return pl2SpptClient.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return pl2SpptClient.getMaxValue() * 0.8;
	}
	if (profile == PerformanceProfile::QUIET) {
		return pl2SpptClient.getMaxValue() * 0.6;
	}

	return pl2SpptClient.getCurrentValue();
}
#endif

#ifdef PPT_PL3_FPPT
int PerformanceService::pl3Fppt(PerformanceProfile profile) {
	if (!acBoost()) {
		return pl1Spl(profile);
	}

	if (profile == PerformanceProfile::PERFORMANCE) {
		return pl3FpptClient.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return pl3FpptClient.getMaxValue() * 0.8;
	}
	if (profile == PerformanceProfile::QUIET) {
		return pl3FpptClient.getMaxValue() * 0.6;
	}

	return pl3FpptClient.getCurrentValue();
}
#endif

#ifdef NV_BOOST
int PerformanceService::nvBoost(PerformanceProfile profile) {
	if (profile == PerformanceProfile::PERFORMANCE) {
		return nvBoostClient.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return (nvBoostClient.getMaxValue() + nvBoostClient.getMinValue()) / 2;
	}
	if (profile == PerformanceProfile::QUIET) {
		return nvBoostClient.getMinValue();
	}

	return nvBoostClient.getCurrentValue();
}
#endif

#ifdef NV_THERMAL
int PerformanceService::acNvTemp() {
	return nvTempClient.getMaxValue();
}

int PerformanceService::batteryNvTemp(PerformanceProfile profile) {
	if (profile == PerformanceProfile::PERFORMANCE) {
		return nvTempClient.getMaxValue();
	}
	if (profile == PerformanceProfile::BALANCED) {
		return (nvTempClient.getMaxValue() + nvTempClient.getMinValue()) / 2;
	}
	if (profile == PerformanceProfile::QUIET) {
		return nvTempClient.getMinValue();
	}

	return nvTempClient.getCurrentValue();
}
#endif

#ifdef SCALING_GOVERNOR
CpuGovernor PerformanceService::acGovernor(PerformanceProfile profile) {
	if (profile == PerformanceProfile::PERFORMANCE) {
		return CpuGovernor::PERFORMANCE;
	}
	return CpuGovernor::POWERSAVE;
}

CpuGovernor PerformanceService::batteryGovernor() {
	return CpuGovernor::POWERSAVE;
}
#endif

int PerformanceService::acTdpToBatteryTdp(int tdp, int minTdp) {
	return std::max(minTdp, static_cast<int>(std::round(tdp * 0.6)));
}

std::vector<std::string> PerformanceService::getAvailableSchedulers() {
	return availableSchedulers;
}

std::string PerformanceService::getCurrentScheduler() {
	return currentScheduler;
}

std::string PerformanceService::getDefaultScheduler() {
	return defaultScheduler;
}

void PerformanceService::setScheduler(std::string scheduler, bool temporal) {
	logger->info("Applying {} scheduler", scheduler);
	Logger::add_tab();

	if (scheduler == currentScheduler) {
		logger->info("Scheduler already applied");
	} else {
		auto t0 = TimeUtils::now();
		if (scheduler == "EEVDF" || scheduler == "BORE") {
			if (currentScheduler == "EEVDF" || currentScheduler == "BORE") {
				if (scxCtlClient.available()) {
					scxCtlClient.stop();
				}
			}
		}

		if (scheduler == "EEVDF") {
			if (schedBoreClient.available()) {
				schedBoreClient.write("0");
			}
		} else if (scheduler == "BORE") {
			schedBoreClient.write("1");
		} else {
			scxCtlClient.start(StringUtils::toLowerCase(scheduler), onBattery);
		}

		currentScheduler = scheduler;

		if (!temporal) {
			configuration.getConfiguration().platform.performance.scheduler = scheduler;
			configuration.saveConfig();
		}

		auto t1 = TimeUtils::now();
		logger->info("Scheduler applied after {} seconds", TimeUtils::format_seconds(TimeUtils::getTimeDiff(t0, t1)));
	}
	Logger::rem_tab();

	eventBus.emitScheduler(scheduler);
}

std::vector<std::string> PerformanceService::getAvailableSsdSchedulers() {
	if (!ssdSchedulerClient.available()) {
		return {};
	}

	return ssdSchedulerClient.getAvailableSchedulers();
}

std::optional<std::string> PerformanceService::getCurrentSsdScheduler() {
	if (!ssdSchedulerClient.available()) {
		return std::nullopt;
	}

	return currentSsdScheduler;
}

void PerformanceService::setSsdScheduler(std::string scheduler, bool temporal) {
	if (!ssdSchedulerClient.available()) {
		return;
	}

	logger->info("Applying {} SSD scheduler", scheduler);
	Logger::add_tab();
	if (scheduler == currentSsdScheduler) {
		logger->info("Scheduler already applied");
	} else {
		auto t0 = TimeUtils::now();
		ssdSchedulerClient.setSchedulers(scheduler);
		currentSsdScheduler = scheduler;

		if (!temporal) {
			configuration.getConfiguration().platform.performance.ssdScheduler = scheduler;
			configuration.saveConfig();
		}
		auto t1 = TimeUtils::now();
		logger->info("Scheduler applied after {} seconds", TimeUtils::format_seconds(TimeUtils::getTimeDiff(t0, t1)));
	}
	Logger::rem_tab();

	eventBus.emitSsdScheduler(scheduler);
}

#ifdef FAN_CONTROL
std::vector<std::string> PerformanceService::getFans() {
	return asusCtlClient.getFans(getPlatformProfile(actualProfile));
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

	for (const auto& [fan, curve] : curves) {
		configuration.getConfiguration().platform.curves[profile][fan].current = curve.toData();
	}
	configuration.saveConfig();

	Logger::rem_tab();
	logger->info("Fan curved updated succesfully");
}

void PerformanceService::restoreFanCurves() {
	logger->info("Restoring fan curve for {}", toName(actualProfile));
	Logger::add_tab();
	setFanCurves(actualProfile);
	Logger::rem_tab();
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
					curve.normalize();
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
#endif