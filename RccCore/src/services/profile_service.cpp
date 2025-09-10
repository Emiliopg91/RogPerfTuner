#include "../../include/services/profile_service.hpp"

#include "../../include/configuration/configuration.hpp"
#include "../../include/events/events.hpp"
#include "../../include/models/performance/cpu_governor.hpp"
#include "../../include/models/performance/power_profile.hpp"
#include "../../include/models/performance/ssd_scheduler.hpp"
#include "../../include/translator/translator.hpp"
#include "../../include/utils/profile_utils.hpp"

ProfileService::ProfileService() {
	logger.info("Initializing ProfileService");
	Logger::add_tab();

	currentProfile = configuration.getConfiguration().platform.profiles.profile;

	if (uPowerClient.available()) {
		onBattery		 = uPowerClient.isOnBattery();
		std::string mode = onBattery ? "battery" : "AC";
		logger.info("Laptop on " + mode + " mode");
	}

	if (platformClient.available()) {
		platformClient.setChangePlatformProfileOnAc(false);
		platformClient.setChangePlatformProfileOnBattery(false);
		platformClient.setPlatformProfileLinkedEpp(true);
	}

	eventBus.on_without_data(Events::HARDWARE_SERVICE_ON_BATTERY, [this]() {
		onBattery = uPowerClient.isOnBattery();
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
		logger.info("Setting " + profileName + " profile");
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
			logger.info("Profile setted after " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()) + " ms");
			std::unordered_map<std::string, std::any> values = {
				{"profile", StringUtils::toLowerCase(translator.translate("label.profile." + profileName))}};
			toaster.showToast(translator.translate("profile.applied", values));
			eventBus.emit_event(Events::PROFILE_SERVICE_ON_PROFILE, {profile});
		} catch (std::exception e) {
			Logger::rem_tab();
		}
	} else {
		logger.info("Profile " + StringUtils::toLowerCase(profileName) + " already setted");
	}
}

void ProfileService::setPlatformProfile(const PerformanceProfile& profile) {
	if (platformClient.available()) {
		auto platformProfile = ProfileUtils::platformProfile(profile);
		logger.info("Platform profile: {}", platformProfile.toName());
		Logger::add_tab();
		try {
			platformClient.setPlatformProfile(platformProfile);
			platformClient.setEnablePptGroup(true);
		} catch (std::exception e) {
			logger.error("Error while setting platform profile: {}", e.what());
		}
		Logger::rem_tab();
	}
}

void ProfileService::setFanCurves(const PerformanceProfile& profile) {
	if (fanCurvesClient.available()) {
		auto platformProfile = ProfileUtils::platformProfile(profile);
		logger.info("Fan profile: {}", platformProfile.toName());
		Logger::add_tab();
		try {
			fanCurvesClient.resetProfileCurve(platformProfile);
			fanCurvesClient.setCurveToDefaults(platformProfile);
			fanCurvesClient.setFanCurveEnabled(platformProfile);
		} catch (std::exception e) {
			logger.error("Error while setting fan curve: {}", std::string(e.what()));
		}
		Logger::rem_tab();
	}
}

void ProfileService::setBoost(const PerformanceProfile&) {
	if (boostControlClient.available()) {
		bool enabled = onBattery ? ProfileUtils::batteryBoost() : ProfileUtils::acBoost();
		logger.info("CPU boost: {}", enabled ? "ON" : "OFF");
		Logger::add_tab();
		try {
			boostControlClient.set_boost(enabled);
		} catch (std::exception e) {
			logger.error("Error while setting CPU boost: {}", std::string(e.what()));
		}
		Logger::rem_tab();
	}
}

void ProfileService::setSsdScheduler(const PerformanceProfile& profile) {
	if (ssdSchedulerClient.available()) {
		SsdScheduler ssdScheduler = ProfileUtils::ssdQueueScheduler(profile);
		logger.info("SSD scheduler: {}", ssdScheduler.toName());
		Logger::add_tab();
		try {
			ssdSchedulerClient.setScheduler(ssdScheduler);
		} catch (std::exception e) {
			logger.error("Error while setting SSD scheduler: {}", std::string(e.what()));
		}
		Logger::rem_tab();
	}
}

void ProfileService::setCpuGovernor(const PerformanceProfile& profile) {
	if (cpuPowerClient.available()) {
		CpuGovernor cpuGovernor = onBattery ? ProfileUtils::batteryGovernor() : ProfileUtils::acGovernor(profile);
		logger.info("CPU governor: {}", cpuGovernor.toName());
		Logger::add_tab();
		try {
			cpuPowerClient.setGovernor(cpuGovernor);
		} catch (std::exception e) {
			logger.error("Error while setting CPU governor: {}", std::string(e.what()));
		}
		Logger::rem_tab();
	}
}

void ProfileService::setPowerProfile(const PerformanceProfile& profile) {
	if (powerProfileClient.available()) {
		PowerProfile powerProfile = ProfileUtils::powerProfile(profile);
		logger.info("Power profile: {}", powerProfile.toName());
		Logger::add_tab();
		try {
			powerProfileClient.setPowerProfile(powerProfile);
		} catch (std::exception e) {
			logger.error("Error while setting power profile: {}", std::string(e.what()));
		}
		Logger::rem_tab();
	}
}

void ProfileService::setTdps(const PerformanceProfile& profile) {
	if (pl1SpdClient.available()) {
		logger.info("TDP values");
		Logger::add_tab();

		auto pl1 = onBattery ? ProfileUtils::batteryIntelPl1Spl(profile) : ProfileUtils::acIntelPl1Spl(profile);
		logger.info("PL1: " + std::to_string(pl1) + "W");
		try {
			pl1SpdClient.setCurrentValue(pl1);
			if (pl2SpptClient.available()) {
				auto pl2 = onBattery ? ProfileUtils::batteryIntelPl2Sppt(profile) : ProfileUtils::acIntelPl2Sppt(profile);
				logger.info("PL2: " + std::to_string(pl2) + "W");
				pl2SpptClient.setCurrentValue(pl2);
			}
		} catch (std::exception e) {
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
				auto nvb = onBattery ? ProfileUtils::batteryNvBoost(profile) : ProfileUtils::acNvBoost(profile);
				logger.info("Dynamic Boost: " + std::to_string(nvb) + "W");
				nvBoostClient.setCurrentValue(nvb);
			} catch (std::exception e) {
				logger.info("Error setting Nvidia Boost");
			}
		}

		if (nvTempClient.available()) {
			try {
				auto nvt = onBattery ? ProfileUtils::batteryNvTemp(profile) : ProfileUtils::acNvTemp();
				logger.info("Throttle temp: " + std::to_string(nvt) + "ºC");
				nvTempClient.setCurrentValue(nvt);
			} catch (std::exception e) {
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
	auto nextProfile = ProfileUtils::nextPerformanceProfile(currentProfile);
	setPerformanceProfile(nextProfile);
	return nextProfile;
}
