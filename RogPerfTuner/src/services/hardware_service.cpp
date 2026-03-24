#include "services/hardware_service.hpp"

#include <exception>
#include <string>

#include "framework/utils/file_utils.hpp"
#include "framework/utils/string_utils.hpp"
#include "framework/utils/time_utils.hpp"
#include "models/hardware/gpu_brand.hpp"
#include "services/open_rgb_service.hpp"
#include "utils/event_bus_wrapper.hpp"

HardwareService::HardwareService() : Loggable("HardwareService") {
	logger->info("Initializing HardwareService");
	Logger::add_tab();

	logger->info("Detected CPU");
	Logger::add_tab();
	logger->info(CPU_NAME);

	Logger::add_tab();
#ifdef PPT_PL1_SPL
	logger->info("TDP control available");
#endif
#ifdef BOOST_CONTROL
	logger->info("Boost control available");
#endif
	Logger::rem_tab();

	Logger::rem_tab();

#ifdef GPU_NAME
	logger->info("Detected GPU");
	Logger::add_tab();
	logger->info(GPU_NAME);

	auto brand = fromString<GpuBrand>(GPU_BRAND);

	if (FileUtils::exists(Constants::LIB_VK_DIR)) {
		FileUtils::remove(Constants::LIB_VK_DIR);
	}
	FileUtils::mkdirs(Constants::LIB_VK_DIR);

	if (FileUtils::exists(Constants::LIB_OCL_DIR)) {
		FileUtils::remove(Constants::LIB_OCL_DIR);
	}
	FileUtils::mkdirs(Constants::LIB_OCL_DIR);

	std::string env = std::string(GPU_ENV) + " ";
	auto icdName	= std::string(GPU_BRAND);
	if (strcmp(GPU_BRAND, "amd") == 0) {
		icdName = "radeon";
	}
	std::vector<std::string> vkIcd;
	std::vector<std::string> vkIcdVariants = {icdName + "_icd.json", icdName + "_icd.i686.json", icdName + "_icd.x86_64.json"};
	for (auto var : vkIcdVariants) {
		if (FileUtils::exists(Constants::USR_SHARE_VK_DIR + var)) {
			FileUtils::copy(Constants::USR_SHARE_VK_DIR + var, Constants::LIB_VK_DIR + var);
			vkIcd.emplace_back(Constants::LIB_VK_DIR + var);
		}
	}
	if (!vkIcd.empty()) {
		std::ostringstream oss;

		for (size_t i = 0; i < vkIcd.size(); ++i) {
			if (i > 0) {
				oss << ":";
			}
			oss << vkIcd[i];
		}

		env = env + "VK_ICD_FILENAMES=" + oss.str() + " ";
	}

	auto ocdName = GPU_BRAND;
	if (strcmp(GPU_BRAND, "amd") == 0) {
		ocdName = "amdocl64";
	}

	if (FileUtils::exists(Constants::USR_SHARE_OCL_DIR + ocdName + ".icd")) {
		FileUtils::copy(Constants::USR_SHARE_OCL_DIR + ocdName + ".icd", Constants::LIB_OCL_DIR + ocdName + ".icd");
		env = env + "OCL_ICD_FILENAMES=" + Constants::LIB_OCL_DIR + ocdName + ".icd" + " ";
	}
	env					  = StringUtils::trim(env);
	gpus[toString(brand)] = env;

	Logger::add_tab();
#ifdef NV_BOOST
	logger->info("Dynamic boost control available");
#endif
#ifdef NV_THERMAL
	logger->info("Throttle temperature control available");
#endif
	Logger::rem_tab();
#endif

#ifdef BAT_LIMIT
	logger->info("Getting battery charge limit");
	Logger::add_tab();
	charge_limit = configuration.getConfiguration().platform.chargeLimit;
	batteryChargeLimitClient.setChargeLimit(charge_limit);
	logger->info(std::to_string(toInt(charge_limit)) + "%");
	Logger::rem_tab();
#endif

	setupDeviceLoop();

	runningGames = 0;
#ifdef PANEL_OD
	setPanelOverdrive(false);
#endif

#ifdef BAT_STATUS
	onBattery = batteryStatusClient.isOnBattery();
	batteryStatusClient.onBatteryChange([this](CallbackAnyParam data) {
		this->onBatteryEvent(std::any_cast<bool>(data));
	});
#endif

	Logger::rem_tab();
}

void HardwareService::setupDeviceLoop() {
	connectedDevices = std::get<0>(udevClient.compare_connected_devs({}, [this](const UsbIdentifier& dev) {
		return !openRgbService.getDeviceName(dev).empty();
	}));
	eventBus.onDeviceEvent([this]() {
		onDeviceEvent();
	});
}

void HardwareService::onDeviceEvent() {
	auto [current, added, removed] = udevClient.compare_connected_devs(connectedDevices, [this](const UsbIdentifier& dev) {
		return !openRgbService.getDeviceName(dev).empty();
	});

	if (added.size() > 0) {
		logger->info("Added compatible device(s):");
		Logger::add_tab();
		for (auto dev : added) {
			logger->info(openRgbService.getDeviceName(dev));
		}
		Logger::rem_tab();
	}

	if (removed.size() > 0) {
		logger->info("Removed compatible device(s):");
		Logger::add_tab();
		for (auto dev : removed) {
			logger->info(openRgbService.getDeviceName(dev));
		}
		Logger::rem_tab();
	}

	if (added.size() > 0) {
		eventBus.emitUsbAdded();
	} else if (removed.size() > 0) {
		eventBus.emitUsbRemoved(removed);
	}

	connectedDevices = current;
}

#ifdef BAT_LIMIT
BatteryThreshold HardwareService::getChargeThreshold() {
	return charge_limit;
}

void HardwareService::setChargeThreshold(const BatteryThreshold& threshold) {
	std::lock_guard<std::mutex> lock(actionMutex);
	if (charge_limit != threshold) {
		logger->info("Setting charge limit to {}%", toInt(threshold));
		auto t0 = TimeUtils::now();
		batteryChargeLimitClient.setChargeLimit(threshold);
		auto t1 = TimeUtils::now();

		charge_limit										  = threshold;
		configuration.getConfiguration().platform.chargeLimit = threshold;
		configuration.saveConfig();

		logger->info("Charge limit setted after {} seconds", TimeUtils::format_seconds(TimeUtils::getTimeDiff(t0, t1)));

		toaster.showToast(translator.translate("applied.battery.threshold", {{"value", std::to_string(toInt(threshold))}}));
		eventBus.emitChargeThreshold(threshold);
	}
}
#endif

void HardwareService::onBatteryEvent(const bool& onBat, const bool& muted) {
	onBattery = onBat;

#ifdef PANEL_OD
	setPanelOverdrive(runningGames > 0 && !onBattery);
#endif

	if (runningGames == 0) {
		if (!muted) {
			std::string t1 = onBattery ? "un" : "";
			std::string t2 = !onBattery ? "dis" : "";
			logger->info("AC {}plugged, battery {}engaged", t1, t2);
			Logger::add_tab();
		}
		eventBus.emitBattery(onBat);
		if (!muted) {
			Logger::rem_tab();
		}
	}
}

#ifdef PANEL_OD
void HardwareService::setPanelOverdrive(const bool& enable) {
	logger->info("Panel Overdrive: {}", enable ? "Enabled" : "Disabled");
	Logger::add_tab();
	try {
		panelOverdriveClient.setCurrentValue(enable);
	} catch (std::exception& e) {
		logger->error(e.what());
	}
	Logger::rem_tab();
}
#endif

std::unordered_map<std::string, std::string> HardwareService::getGpuSelectorEnv(const std::string& gpu) {
	std::unordered_map<std::string, std::string> env;

	auto it = gpus.find(gpu);
	if (it != gpus.end()) {
		auto gpuEnv = it->second;

		std::istringstream ss(gpuEnv);
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
			env[key] = val;
		}
	}

	return env;
}

std::unordered_map<std::string, std::string> HardwareService::getGpus() {
	return gpus;
}

#ifdef BOOT_SOUND
bool HardwareService::getBootSound() {
	return bootSoundClient.getCurrentValue();
}

void HardwareService::setBootSound(bool enable) {
	if (enable == getBootSound()) {
		return;
	}

	std::lock_guard<std::mutex> lock(actionMutex);
	logger->info("Setting boot sound: {}", enable ? "Enabled" : "Disabled");
	Logger::add_tab();
	auto t0 = TimeUtils::now();
	bootSoundClient.setCurrentValue(enable);
	auto t1 = TimeUtils::now();
	Logger::rem_tab();
	logger->info("Boot sound setted after {} seconds", TimeUtils::format_seconds(TimeUtils::getTimeDiff(t0, t1)));
	eventBus.emitBootSound(enable);
}
#endif