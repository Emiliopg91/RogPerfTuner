#include "../../include/services/hardware_service.hpp"

#include "../../include/clients/dbus/asus/armoury/intel/pl1_spd_client.hpp"
#include "../../include/clients/dbus/asus/armoury/nvidia/nv_boost_client.hpp"
#include "../../include/clients/dbus/asus/armoury/nvidia/nv_temp_client.hpp"
#include "../../include/clients/dbus/asus/armoury/other/panel_overdrive_client.hpp"
#include "../../include/clients/dbus/asus/core/platform_client.hpp"
#include "../../include/clients/dbus/linux/notifications_client.hpp"
#include "../../include/clients/dbus/linux/power_management_kb_brightness.hpp"
#include "../../include/clients/dbus/linux/upower_client.hpp"
#include "../../include/clients/file/boost_control_client.hpp"
#include "../../include/clients/file/cpuinfo_client.hpp"
#include "../../include/clients/file/ssd_scheduler_client.hpp"
#include "../../include/clients/shell/lsusb_client.hpp"
#include "../../include/clients/shell/switcherooctl_client.hpp"
#include "../../include/clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../../include/gui/toaster.hpp"
#include "../../include/models/hardware/battery_charge_threshold.hpp"
#include "../../include/models/hardware/cpu_brand.hpp"
#include "../../include/models/hardware/gpu_brand.hpp"
#include "../../include/services/open_rgb_service.hpp"
#include "../../include/translator/translator.hpp"
#include "RccCommons.hpp"

HardwareService::HardwareService() {
	logger.info("Initializing HardwareService");
	logger.add_tab();

	logger.info("Detecting CPU");
	logger.add_tab();
	std::string cpuinfo_out = CPUInfoClient::getInstance().read(5);
	if (StringUtils::isSubstring("GenuineIntel", cpuinfo_out)) {
		cpu		   = CpuBrand::Enum::INTEL;
		auto lines = StringUtils::splitLines(cpuinfo_out);
		auto line  = lines[lines.size() - 1];
		auto pos   = line.find(":");
		if (pos != std::string::npos) {
			line = line.substr(pos + 2);
			logger.info(line);
		}
	}

	logger.add_tab();
	if (cpu == CpuBrand::Enum::INTEL) {
		if (Pl1SpdClient::getInstance().available()) {
			logger.info("TDP control available");
		}
		if (BoostControlClient::getInstance().available()) {
			logger.info("Boost control available");
		}
	}
	logger.rem_tab();
	logger.rem_tab();

	logger.info("Detecting GPUs");
	logger.add_tab();
	auto detected_gpus = SwitcherooCtlClient::getInstance().getGpus();
	for (auto gpu : detected_gpus) {
		logger.info(gpu.name);
		if (!gpu.default_flag) {
			auto brand = static_cast<GpuBrand>(GpuBrand::fromString(StringUtils::toLowerCase(StringUtils::split(gpu.name, ' ')[0])));
			std::string env;
			if (!gpu.environment.empty()) {
				for (auto gpu_env : gpu.environment)
					env = env + gpu_env + " ";
			}

			if (FileUtils::exists(Constants::LIB_VK_DIR))
				FileUtils::remove(Constants::LIB_VK_DIR);
			FileUtils::mkdirs(Constants::LIB_VK_DIR);

			if (FileUtils::exists(Constants::LIB_OCL_DIR))
				FileUtils::remove(Constants::LIB_OCL_DIR);
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
					if (i > 0)
						oss << ":";
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

			logger.add_tab();
			if (brand == GpuBrand::Enum::NVIDIA) {
				if (NvBoostClient::getInstance().available()) {
					logger.info("Dynamic boost control available");
				}
				if (NvTempClient::getInstance().available()) {
					logger.info("Throttle temperature control available");
				}
			}

			logger.rem_tab();
		}
	}
	logger.rem_tab();

	if (SsdSchedulerClient::getInstance().available()) {
		logger.info("Getting available SSD schedulers");
		logger.add_tab();
		ssd_schedulers = SsdSchedulerClient::getInstance().get_schedulers();
		for (auto sched : ssd_schedulers)
			logger.info(sched.toString());
		logger.rem_tab();
	}

	if (PlatformClient::getInstance().available()) {
		logger.info("Getting battery charge limit");
		logger.add_tab();
		charge_limit = PlatformClient::getInstance().getBatteryLimit();
		logger.info(std::to_string(charge_limit.toInt()) + "%");
		logger.rem_tab();
	}

	setupDeviceLoop();

	runningGames = 0;

	if (UPowerClient::getInstance().available()) {
		onBattery = UPowerClient::getInstance().isOnBattery();
		UPowerClient::getInstance().onBatteryChange([this](CallbackParam data) { this->onBatteryEvent(std::any_cast<bool>(data[0])); });
	}

	if (PMKeyboardBrightness::getInstance().available()) {
		PMKeyboardBrightness::getInstance().onBrightnessChange([this]() {
			if (PMKeyboardBrightness::getInstance().getKeyboardBrightness() == 0) {
				PMKeyboardBrightness::getInstance().setKeyboardBrightnessSilent(2);
			}
		});
	}

	logger.rem_tab();
}

void HardwareService::setupDeviceLoop() {
	auto& udevClient = LsUsbClient::getInstance();
	connectedDevices = std::get<0>(
		udevClient.compare_connected_devs({}, [](const UsbIdentifier& dev) { return !OpenRgbService::getInstance().getDeviceName(dev).empty(); }));
	EventBus::getInstance().on_without_data(Events::UDEV_CLIENT_DEVICE_EVENT, [this, &udevClient]() {
		auto [current, added, removed] = udevClient.compare_connected_devs(
			connectedDevices, [](const UsbIdentifier& dev) { return !OpenRgbService::getInstance().getDeviceName(dev).empty(); });

		if (added.size() > 0) {
			logger.info("Added compatible device(s):");
			logger.add_tab();
			for (auto dev : added) {
				logger.info(OpenRgbService::getInstance().getDeviceName(dev));
			}
			logger.rem_tab();
		}

		if (removed.size() > 0) {
			logger.info("Removed compatible device(s):");
			logger.add_tab();
			for (auto dev : removed) {
				logger.info(OpenRgbService::getInstance().getDeviceName(dev));
			}
			logger.rem_tab();
		}

		if (removed.size() > 0 && added.size() == 0) {
			for (auto dev : removed) {
				OpenRgbService::getInstance().disableDevice(dev);
			}
		} else if (removed.size() > 0 || added.size() > 0) {
			EventBus::getInstance().emit_event(Events::HARDWARE_SERVICE_USB_ADDED_REMOVED);
		}

		connectedDevices = current;
	});
}

BatteryThreshold HardwareService::getChargeThreshold() {
	return charge_limit;
}

void HardwareService::setChargeThreshold(const BatteryThreshold& threshold) {
	std::lock_guard<std::mutex> lock(actionMutex);
	if (charge_limit != threshold) {
		logger.info("Setting charge limit to " + std::to_string(threshold.toInt()) + "%");
		auto t0 = std::chrono::high_resolution_clock::now();
		PlatformClient::getInstance().setBatteryLimit(threshold);
		auto t1 = std::chrono::high_resolution_clock::now();

		charge_limit = threshold;
		logger.info("Charge limit setted after " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()) + " ms");

		std::unordered_map<std::string, std::any> replacements = {{"value", threshold.toInt()}};
		Toaster::getInstance().showToast(Translator::getInstance().translate("applied.battery.threshold", replacements));
		EventBus::getInstance().emit_event(Events::HARDWARE_SERVICE_THRESHOLD_CHANGED, {threshold});
	}
}

void HardwareService::onBatteryEvent(const bool& onBat, const bool& muted) {
	onBattery = onBat;

	setPanelOverdrive(runningGames > 0 && !onBattery);

	if (runningGames == 0) {
		if (!muted) {
			std::string t1 = onBattery ? "un" : "";
			std::string t2 = !onBattery ? "dis" : "";
			logger.info("AC " + t1 + "plugged, battery " + t2 + "engaged");
			logger.add_tab();
		}
		EventBus::getInstance().emit_event(Events::HARDWARE_SERVICE_ON_BATTERY);
		if (!muted) {
			logger.rem_tab();
		}
	}
}

void HardwareService::setPanelOverdrive(const bool& enable) {
	if (PanelOverdriveClient::getInstance().available()) {
		logger.info("Setting panel overdrive to {}", enable);
		PanelOverdriveClient::getInstance().setCurrentValue(enable);
	}
}
void HardwareService::renice(const pid_t& pid) {
	Shell::getInstance().run_elevated_command(
		fmt::format("renice -n {} -p {} && ionice -c {} -n {} -p {}", CPU_PRIORITY, pid, IO_CLASS, IO_PRIORITY, pid));
}

std::map<std::string, std::string> HardwareService::getGpuSelectorEnv(const std::string& gpu) {
	std::map<std::string, std::string> env;

	auto it = gpus.find(gpu);
	if (it != gpus.end()) {
		auto gpuEnv = it->second;

		std::istringstream ss(gpuEnv);
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
			env[key] = val;
		}
	}

	return env;
}