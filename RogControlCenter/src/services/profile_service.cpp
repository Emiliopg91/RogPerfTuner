#include "../../include/services/profile_service.hpp"

#include <optional>

#include "../../include/configuration/configuration.hpp"
#include "../../include/events/event_bus.hpp"
#include "../../include/models/performance/cpu_governor.hpp"
#include "../../include/models/performance/power_profile.hpp"
#include "../../include/models/performance/ssd_scheduler.hpp"
#include "../../include/utils/string_utils.hpp"
#include "../../include/utils/time_utils.hpp"

ProfileService::ProfileService() : Loggable("ProfileService") {
	logger.info("Initializing ProfileService");
	Logger::add_tab();

	currentProfile	 = configuration.getConfiguration().performance.profile;
	currentScheduler = configuration.getConfiguration().performance.scheduler;

	if (uPowerClient.available()) {
		onBattery		 = uPowerClient.isOnBattery();
		std::string mode = onBattery ? "battery" : "AC";
		logger.info("Laptop on {} mode", mode);
	}

	if (platformClient.available()) {
		platformClient.setChangePlatformProfileOnAc(false);
		platformClient.setChangePlatformProfileOnBattery(false);
		platformClient.setPlatformProfileLinkedEpp(true);
	}

	eventBus.onBattery([this](bool onBat) {
		onBattery = onBat;
		if (runningGames == 0) {
			if (onBattery) {
				PerformanceProfile p = PerformanceProfile::Enum::QUIET;
				setPerformanceProfile(p, true, true);
			} else {
				restoreProfile();
			}
		}
	});

	eventBus.onApplicationStop([this]() {
		PerformanceProfile p = PerformanceProfile::Enum::PERFORMANCE;
		this->setPerformanceProfile(p, true, true);
	});

	restore();

	Logger::rem_tab();
}

void ProfileService::renice(const pid_t& pid) {
	logger.info("Renicing process {}", pid);
	Logger::add_tab();
	shell.run_elevated_command(fmt::format("renice -n {} -p {} && ionice -c {} -n {} -p {}", CPU_PRIORITY, pid, IO_CLASS, IO_PRIORITY, pid));
	Logger::rem_tab();
}

PerformanceProfile ProfileService::getPerformanceProfile() {
	return currentProfile;
}

void ProfileService::setPerformanceProfile(PerformanceProfile& profile, const bool& temporal, const bool& force, const bool& silent) {
	std::lock_guard<std::mutex> lock(actionMutex);
	std::string profileName = profile.toName();

	if (profile != currentProfile || force) {
		logger.info("Setting {} profile", profileName);
		Logger::add_tab();
		try {
			setPlatformProfile(profile);
			//  setFanCurves(profile);
			setBoost(profile);
			setCpuGovernor(profile);
			setSsdScheduler(profile);
			setPowerProfile(profile);
			setTdps(profile);
			setTgp(profile);

			currentProfile = profile;
			if (!temporal) {
				configuration.getConfiguration().performance.profile = profile;
				configuration.saveConfig();
			}

			Logger::rem_tab();
			logger.info("Profile setted succesfully");

			if (silent) {
				toaster.showToast(translator.translate(
					"profile.applied", {{"profile", StringUtils::toLowerCase(translator.translate("label.profile." + profileName))}}));
			}

			eventBus.emitPerformanceProfile(profile);
		} catch (std::exception& e) {
			Logger::rem_tab();
		}
	} else {
		logger.info("Profile {} already setted", StringUtils::toLowerCase(profileName));
	}
}

void ProfileService::setPlatformProfile(PerformanceProfile& profile) {
	if (platformClient.available()) {
		auto platformProfile = profile.getPlatformProfile();
		logger.info("Platform profile: {}", platformProfile.toName());
		Logger::add_tab();
		try {
			platformClient.setPlatformProfile(platformProfile);
			platformClient.setEnablePptGroup(true);
		} catch (std::exception& e) {
			logger.error("Error while setting platform profile: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void ProfileService::setFanCurves(PerformanceProfile& profile) {
	if (fanCurvesClient.available()) {
		auto platformProfile = profile.getPlatformProfile();
		logger.info("Fan profile: {}", platformProfile.toName());
		Logger::add_tab();
		try {
			fanCurvesClient.resetProfileCurve(platformProfile);
			fanCurvesClient.setCurveToDefaults(platformProfile);
			fanCurvesClient.setFanCurveEnabled(platformProfile);
		} catch (std::exception& e) {
			logger.error("Error while setting fan curve: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void ProfileService::setBoost(const PerformanceProfile&) {
	if (boostControlClient.available()) {
		bool enabled = onBattery ? batteryBoost() : acBoost();
		logger.info("CPU boost: {}", enabled ? "ON" : "OFF");
		Logger::add_tab();
		try {
			boostControlClient.set_boost(enabled);
		} catch (std::exception& e) {
			logger.error("Error while setting CPU boost: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void ProfileService::setSsdScheduler(PerformanceProfile& profile) {
	if (ssdSchedulerClient.available()) {
		SsdScheduler ssdScheduler = profile.getSsdQueueScheduler();
		logger.info("SSD scheduler: {}", ssdScheduler.toName());
		Logger::add_tab();
		try {
			ssdSchedulerClient.setScheduler(ssdScheduler);
		} catch (std::exception& e) {
			logger.error("Error while setting SSD scheduler: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void ProfileService::setCpuGovernor(const PerformanceProfile& profile) {
	if (cpuPowerClient.available()) {
		CpuGovernor cpuGovernor = onBattery ? batteryGovernor() : acGovernor(profile);
		logger.info("CPU governor: {}", cpuGovernor.toName());
		Logger::add_tab();
		try {
			cpuPowerClient.setGovernor(cpuGovernor);
		} catch (std::exception& e) {
			logger.error("Error while setting CPU governor: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void ProfileService::setPowerProfile(PerformanceProfile& profile) {
	if (powerProfileClient.available()) {
		PowerProfile powerProfile = profile.getPowerProfile();
		logger.info("Power profile: {}", powerProfile.toName());
		Logger::add_tab();
		try {
			powerProfileClient.setPowerProfile(powerProfile);
		} catch (std::exception& e) {
			logger.error("Error while setting power profile: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void ProfileService::setTdps(const PerformanceProfile& profile) {
	if (pl1SpdClient.available()) {
		logger.info("TDP values");
		Logger::add_tab();

		auto pl1 = onBattery ? batteryIntelPl1Spl(profile) : acIntelPl1Spl(profile);
		logger.info("PL1: {}W", pl1);
		try {
			pl1SpdClient.setCurrentValue(pl1);
			TimeUtils::sleep(25);
			if (pl2SpptClient.available()) {
				auto pl2 = onBattery ? batteryIntelPl2Sppt(profile) : acIntelPl2Sppt(profile);
				logger.info("PL2: {}W", pl2);
				TimeUtils::sleep(25);
				pl2SpptClient.setCurrentValue(pl2);
			}
		} catch (std::exception& e) {
			logger.info("Error setting CPU TDPs");
		}

		Logger::rem_tab();
	}
}

void ProfileService::setTgp(const PerformanceProfile& profile) {
	if (nvTempClient.available() || nvBoostClient.available()) {
		logger.info("Nvidia GPU");
		Logger::add_tab();

		if (nvBoostClient.available()) {
			try {
				auto nvb = onBattery ? batteryNvBoost(profile) : acNvBoost(profile);
				logger.info("Dynamic Boost: {}W", nvb);
				nvBoostClient.setCurrentValue(nvb);
				TimeUtils::sleep(25);
			} catch (std::exception& e) {
				logger.info("Error setting Nvidia Boost");
			}
		}

		if (nvTempClient.available()) {
			try {
				auto nvt = onBattery ? batteryNvTemp(profile) : acNvTemp();
				logger.info("Throttle temp: {}ºC", nvt);
				nvTempClient.setCurrentValue(nvt);
			} catch (std::exception& e) {
				logger.info("Error setting Nvidia TGP");
			}
		}

		Logger::rem_tab();
	}
}

void ProfileService::restore() {
	restoreProfile();
	restoreScheduler();
}

void ProfileService::restoreProfile() {
	setPerformanceProfile(configuration.getConfiguration().performance.profile, false, true);
}

void ProfileService::restoreScheduler() {
	setScheduler(configuration.getConfiguration().performance.scheduler);
}

PerformanceProfile ProfileService::nextPerformanceProfile() {
	auto nextProfile = currentProfile.getNextPerformanceProfile();
	setPerformanceProfile(nextProfile);
	return nextProfile;
}

int ProfileService::acIntelPl1Spl(PerformanceProfile profile) {
	auto& client = Pl1SpdClient::getInstance();

	if (profile == PerformanceProfile::Enum::PERFORMANCE) {
		return client.getMaxValue();
	}
	if (profile == PerformanceProfile::Enum::BALANCED) {
		return client.getMaxValue() * 0.6;
	}
	if (profile == PerformanceProfile::Enum::QUIET) {
		return client.getMaxValue() * 0.4;
	}

	return client.getCurrentValue();
}

int ProfileService::batteryIntelPl1Spl(PerformanceProfile profile) {
	int acVal	 = acIntelPl1Spl(profile);
	auto& client = Pl1SpdClient::getInstance();
	return acTdpToBatteryTdp(acIntelPl1Spl(profile), client.getMinValue());
}

int ProfileService::acIntelPl2Sppt(PerformanceProfile profile) {
	auto& client = Pl2SpptClient::getInstance();

	if (!acBoost()) {
		return acIntelPl1Spl(profile);
	}

	if (profile == PerformanceProfile::Enum::PERFORMANCE) {
		return client.getMaxValue();
	}
	if (profile == PerformanceProfile::Enum::BALANCED) {
		return client.getMaxValue() * 0.8;	// modificar
	}
	if (profile == PerformanceProfile::Enum::QUIET) {
		return client.getMaxValue() * 0.6;	// modificar
	}

	return client.getCurrentValue();
}

int ProfileService::batteryIntelPl2Sppt(PerformanceProfile profile) {
	int acVal	 = acIntelPl2Sppt(profile);
	auto& client = Pl2SpptClient::getInstance();
	return acTdpToBatteryTdp(acIntelPl2Sppt(profile), client.getMinValue());
}

int ProfileService::acNvBoost(PerformanceProfile profile) {
	auto& client = NvBoostClient::getInstance();

	if (profile == PerformanceProfile::Enum::PERFORMANCE) {
		return client.getMaxValue();
	}
	if (profile == PerformanceProfile::Enum::BALANCED) {
		return (client.getMaxValue() + client.getMinValue()) / 2;
	}
	if (profile == PerformanceProfile::Enum::QUIET) {
		return client.getMinValue();
	}

	return client.getCurrentValue();
}

int ProfileService::batteryNvBoost(PerformanceProfile profile) {
	auto& client = NvBoostClient::getInstance();
	return acTdpToBatteryTdp(acNvBoost(profile), client.getMinValue());
}

int ProfileService::acNvTemp() {
	auto& client = NvTempClient::getInstance();
	return client.getMaxValue();
}

int ProfileService::batteryNvTemp(PerformanceProfile profile) {
	auto& client = NvTempClient::getInstance();

	if (profile == PerformanceProfile::Enum::PERFORMANCE) {
		return client.getMaxValue();
	}
	if (profile == PerformanceProfile::Enum::BALANCED) {
		return (client.getMaxValue() + client.getMinValue()) / 2;
	}
	if (profile == PerformanceProfile::Enum::QUIET) {
		return client.getMinValue();
	}

	return client.getCurrentValue();
}

bool ProfileService::acBoost() {
	return true;
}
bool ProfileService::batteryBoost() {
	return false;
}

CpuGovernor ProfileService::acGovernor(PerformanceProfile profile) {
	if (profile == PerformanceProfile::Enum::PERFORMANCE) {
		return CpuGovernor::Enum::PERFORMANCE;
	}
	return CpuGovernor::Enum::POWERSAVE;
}

CpuGovernor ProfileService::batteryGovernor() {
	return CpuGovernor::Enum::POWERSAVE;
}

int ProfileService::acTdpToBatteryTdp(int tdp, int minTdp) {
	return std::max(minTdp, static_cast<int>(std::round(tdp * 0.6)));
}

std::vector<std::string> ProfileService::getAvailableSchedulers() {
	if (!scxCtlClient.available()) {
		return {};
	}

	return scxCtlClient.getAvailable();
}

std::optional<std::string> ProfileService::getCurrentScheduler() {
	if (!scxCtlClient.available()) {
		return std::nullopt;
	}

	return currentScheduler;
}

void ProfileService::setScheduler(std::optional<std::string> scheduler) {
	if (!scxCtlClient.available()) {
		return;
	}

	if (scheduler.has_value()) {
		scxCtlClient.start(scheduler.value());
	} else {
		scxCtlClient.stop();
	}

	currentScheduler = scheduler;

	configuration.getConfiguration().performance.scheduler = scheduler;
	configuration.saveConfig();

	eventBus.emitScheduler(scheduler);
}