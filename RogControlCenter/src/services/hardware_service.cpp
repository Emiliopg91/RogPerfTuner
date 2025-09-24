#include "../../include/services/hardware_service.hpp"

#include "../../include/clients/dbus/asus/armoury/intel/pl1_spd_client.hpp"
#include "../../include/clients/dbus/asus/armoury/nvidia/nv_boost_client.hpp"
#include "../../include/clients/dbus/asus/armoury/nvidia/nv_temp_client.hpp"
#include "../../include/clients/dbus/asus/armoury/other/panel_overdrive_client.hpp"
#include "../../include/clients/dbus/asus/core/platform_client.hpp"
#include "../../include/clients/dbus/linux/power_management_kb_brightness.hpp"
#include "../../include/clients/dbus/linux/upower_client.hpp"
#include "../../include/clients/file/boost_control_client.hpp"
#include "../../include/clients/file/cpuinfo_client.hpp"
#include "../../include/clients/file/ssd_scheduler_client.hpp"
#include "../../include/clients/lib/lsusb_client.hpp"
#include "../../include/clients/shell/switcherooctl_client.hpp"
#include "../../include/events/event_bus.hpp"
#include "../../include/gui/toaster.hpp"
#include "../../include/models/hardware/battery_charge_threshold.hpp"
#include "../../include/models/hardware/cpu_brand.hpp"
#include "../../include/models/hardware/gpu_brand.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/translator/translator.hpp"
#include "../../include/utils/string_utils.hpp"
#include "../../include/utils/time_utils.hpp"

HardwareService::HardwareService() : Loggable("HardwareService") {
	logger.info("Initializing HardwareService");
	Logger::add_tab();

	logger.info("Detecting CPU");
	Logger::add_tab();
	std::string cpuinfo_out = cpuInfoClient.read(5);
	if (StringUtils::isSubstring("GenuineIntel", cpuinfo_out)) {
		cpu		   = CpuBrand::Enum::INTEL;
		auto lines = StringUtils::splitLines(cpuinfo_out);
		auto line  = lines[lines.size() - 1];
		auto pos   = line.find(":");
		if (pos != std::string::npos) {
			line = line.substr(pos + 2);
			logger.info("{}", line);
		}
	}

	Logger::add_tab();
	if (cpu == CpuBrand::Enum::INTEL) {
		if (pl1SpdClient.available()) {
			logger.info("TDP control available");
		}
		if (boostControlClient.available()) {
			logger.info("Boost control available");
		}
	}
	Logger::rem_tab();
	Logger::rem_tab();

	logger.info("Detecting GPUs");
	Logger::add_tab();
	auto detected_gpus = switcherooCtlClient.getGpus();
	for (auto gpu : detected_gpus) {
		logger.info("{}", gpu.name);
		if (!gpu.default_flag) {
			auto brand = static_cast<GpuBrand>(GpuBrand::fromString(StringUtils::toLowerCase(StringUtils::split(gpu.name, ' ')[0])));
			std::string env;
			if (!gpu.environment.empty()) {
				for (auto gpu_env : gpu.environment) {
					env = env + gpu_env + " ";
				}
			}

			if (FileUtils::exists(Constants::LIB_VK_DIR)) {
				FileUtils::remove(Constants::LIB_VK_DIR);
			}
			FileUtils::mkdirs(Constants::LIB_VK_DIR);

			if (FileUtils::exists(Constants::LIB_OCL_DIR)) {
				FileUtils::remove(Constants::LIB_OCL_DIR);
			}
			FileUtils::mkdirs(Constants::LIB_OCL_DIR);

			std::vector<std::string> vkIcd;
			std::vector<std::string> vkIcdVariants = {brand.toString() + "_icd.json", brand.toString() + "_icd.i686.json",
													  brand.toString() + "_icd.x86_64.json"};
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

			if (FileUtils::exists(Constants::USR_SHARE_OCL_DIR + brand.toString() + ".icd")) {
				FileUtils::copy(Constants::USR_SHARE_OCL_DIR + brand.toString() + ".icd", Constants::LIB_OCL_DIR + brand.toString() + ".icd");
				env = env + "OCL_ICD_FILENAMES=" + Constants::LIB_OCL_DIR + brand.toString() + ".icd" + " ";
			}
			env					   = StringUtils::trim(env);
			gpus[brand.toString()] = env;

			Logger::add_tab();
			if (brand == GpuBrand::Enum::NVIDIA) {
				if (nvBoostClient.available()) {
					logger.info("Dynamic boost control available");
				}
				if (nvTempClient.available()) {
					logger.info("Throttle temperature control available");
				}
			}

			Logger::rem_tab();
		}
	}
	Logger::rem_tab();

	if (ssdSchedulerClient.available()) {
		logger.info("Getting available SSD schedulers");
		Logger::add_tab();
		ssd_schedulers = ssdSchedulerClient.get_schedulers();
		for (auto sched : ssd_schedulers) {
			logger.info("{}", sched.toString());
		}
		Logger::rem_tab();
	}

	if (platformClient.available()) {
		logger.info("Getting battery charge limit");
		Logger::add_tab();
		charge_limit = configuration.getConfiguration().platform.chargeLimit;
		platformClient.setBatteryLimit(charge_limit);
		logger.info("{} %", charge_limit.toInt());
		Logger::rem_tab();
	}

	setupDeviceLoop();

	runningGames = 0;

	if (uPowerClient.available()) {
		onBattery = uPowerClient.isOnBattery();
		uPowerClient.onBatteryChange([this](CallbackParam data) {
			this->onBatteryEvent(std::any_cast<bool>(data[0]));
		});
	}

	if (pmKeyboardBrightnessClient.available()) {
		pmKeyboardBrightnessClient.onBrightnessChange([this]() {
			if (pmKeyboardBrightnessClient.getKeyboardBrightness() == 0) {
				pmKeyboardBrightnessClient.setKeyboardBrightnessSilent(2);
			}
		});
	}

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
		logger.info("Added compatible device(s):");
		Logger::add_tab();
		for (auto dev : added) {
			logger.info("{}", openRgbService.getDeviceName(dev));
		}
		Logger::rem_tab();
	}

	if (removed.size() > 0) {
		logger.info("Removed compatible device(s):");
		Logger::add_tab();
		for (auto dev : removed) {
			logger.info("{}", openRgbService.getDeviceName(dev));
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

BatteryThreshold HardwareService::getChargeThreshold() {
	return charge_limit;
}

void HardwareService::setChargeThreshold(const BatteryThreshold& threshold) {
	std::lock_guard<std::mutex> lock(actionMutex);
	if (charge_limit != threshold) {
		logger.info("Setting charge limit to {}%", threshold.toInt());
		auto t0 = TimeUtils::now();
		platformClient.setBatteryLimit(threshold);
		auto t1 = TimeUtils::now();

		charge_limit										  = threshold;
		configuration.getConfiguration().platform.chargeLimit = threshold;
		configuration.saveConfig();

		logger.info("Charge limit setted after {} ms", TimeUtils::getTimeDiff(t0, t1));

		toaster.showToast(translator.translate("applied.battery.threshold", {{"value", std::to_string(threshold.toInt())}}));
		eventBus.emitChargeThreshold(threshold);
	}
}

void HardwareService::onBatteryEvent(const bool& onBat, const bool& muted) {
	onBattery = onBat;

	setPanelOverdrive(runningGames > 0 && !onBattery);

	if (runningGames == 0) {
		if (!muted) {
			std::string t1 = onBattery ? "un" : "";
			std::string t2 = !onBattery ? "dis" : "";
			logger.info("AC {}plugged, battery {}engaged", t1, t2);
			Logger::add_tab();
		}
		eventBus.emitBattery(onBat);
		if (!muted) {
			Logger::rem_tab();
		}
	}
}

void HardwareService::setPanelOverdrive(const bool& enable) {
	if (panelOverdriveClient.available()) {
		logger.info("Panel Overdrive: {}", enable ? "Enabled" : "Disabled");
		panelOverdriveClient.setCurrentValue(enable);
	}
}

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