#include "../../include/services/profile_service.hpp"

#include <thread>

#include "../../include/configuration/configuration.hpp"
#include "../../include/events/event_bus.hpp"
#include "../../include/models/performance/cpu_governor.hpp"
#include "../../include/models/performance/power_profile.hpp"
#include "../../include/models/performance/ssd_scheduler.hpp"
#include "../../include/utils/string_utils.hpp"

ProfileService::ProfileService() {
	logger.info("Initializing ProfileService");
	Logger::add_tab();

	currentProfile = configuration.getConfiguration().platform.profiles.profile;

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
				setPerformanceProfile(PerformanceProfile::Enum::QUIET, true, true);
			} else {
				restoreProfile();
			}
		}
	});

	restoreProfile();

	Logger::rem_tab();
}

PerformanceProfile ProfileService::getPerformanceProfile() {
	return currentProfile;
}
void ProfileService::setPerformanceProfile(const PerformanceProfile& profile, const bool& temporal, const bool& force) {
	std::lock_guard<std::mutex> lock(actionMutex);
	std::string profileName = profile.toName();

	if (profile != currentProfile || force) {
		logger.info("Setting {} profile", profileName);
		Logger::add_tab();
		try {
			auto t0 = std::chrono::high_resolution_clock::now();

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
				configuration.getConfiguration().platform.profiles.profile = profile;
				configuration.saveConfig();
			}

			auto t1 = std::chrono::high_resolution_clock::now();
			Logger::rem_tab();
			logger.info("Profile setted after {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
			toaster.showToast(translator.translate("profile.applied",
												   {{"profile", StringUtils::toLowerCase(translator.translate("label.profile." + profileName))}}));
			eventBus.emitPerformanceProfile(profile);
		} catch (std::exception& e) {
			Logger::rem_tab();
		}
	} else {
		logger.info("Profile {} already setted", StringUtils::toLowerCase(profileName));
	}
}

void ProfileService::setPlatformProfile(const PerformanceProfile& profile) {
	if (platformClient.available()) {
		auto platformProfile = getPlatformProfile(profile);
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

void ProfileService::setFanCurves(const PerformanceProfile& profile) {
	if (fanCurvesClient.available()) {
		auto platformProfile = getPlatformProfile(profile);
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

void ProfileService::setSsdScheduler(const PerformanceProfile& profile) {
	if (ssdSchedulerClient.available()) {
		SsdScheduler ssdScheduler = ssdQueueScheduler(profile);
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

void ProfileService::setPowerProfile(const PerformanceProfile& profile) {
	if (powerProfileClient.available()) {
		PowerProfile powerProfile = getPowerProfile(profile);
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
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			if (pl2SpptClient.available()) {
				auto pl2 = onBattery ? batteryIntelPl2Sppt(profile) : acIntelPl2Sppt(profile);
				logger.info("PL2: {}W", pl2);
				std::this_thread::sleep_for(std::chrono::milliseconds(25));
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
				std::this_thread::sleep_for(std::chrono::milliseconds(25));
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

void ProfileService::restoreProfile() {
	setPerformanceProfile(configuration.getConfiguration().platform.profiles.profile, false, true);
}

PerformanceProfile ProfileService::nextPerformanceProfile() {
	auto nextProfile = nextPerformanceProfile(currentProfile);
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

PerformanceProfile ProfileService::nextPerformanceProfile(PerformanceProfile profile) {
	if (profile == PerformanceProfile::Enum::PERFORMANCE) {
		return PerformanceProfile::Enum::QUIET;
	}
	if (profile == PerformanceProfile::Enum::BALANCED) {
		return PerformanceProfile::Enum::PERFORMANCE;
	}
	if (profile == PerformanceProfile::Enum::QUIET) {
		return PerformanceProfile::Enum::BALANCED;
	}
	return profile;
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

PerformanceProfile ProfileService::getGreater(PerformanceProfile profile, const PerformanceProfile& other) {
	if (profile == PerformanceProfile::Enum::PERFORMANCE || other == PerformanceProfile::Enum::PERFORMANCE) {
		return PerformanceProfile::Enum::PERFORMANCE;
	}
	if (profile == PerformanceProfile::Enum::BALANCED || other == PerformanceProfile::Enum::BALANCED) {
		return PerformanceProfile::Enum::BALANCED;
	}
	return PerformanceProfile::Enum::QUIET;
}

PlatformProfile ProfileService::getPlatformProfile(PerformanceProfile profile) {
	if (profile == PerformanceProfile::Enum::QUIET) {
		return PlatformProfile::Enum::LOW_POWER;
	} else if (profile == PerformanceProfile::Enum::BALANCED) {
		return PlatformProfile::Enum::BALANCED;
	} else {
		return PlatformProfile::Enum::PERFORMANCE;
	}
}

PowerProfile ProfileService::getPowerProfile(PerformanceProfile profile) {
	if (profile == PerformanceProfile::Enum::QUIET) {
		return PowerProfile::Enum::POWERSAVER;
	} else if (profile == PerformanceProfile::Enum::BALANCED) {
		return PowerProfile::Enum::BALANCED;
	} else {
		return PowerProfile::Enum::PERFORMANCE;
	}
}

SsdScheduler ProfileService::ssdQueueScheduler(PerformanceProfile profile) {
	if (profile == PerformanceProfile::Enum::QUIET) {
		return SsdScheduler::Enum::NOOP;
	} else {
		return SsdScheduler::Enum::MQ_DEADLINE;
	}
}

int ProfileService::acTdpToBatteryTdp(int tdp, int minTdp) {
	return std::max(minTdp, static_cast<int>(std::round(tdp * 0.6)));
}