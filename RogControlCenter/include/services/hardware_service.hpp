#pragma once

#include <mutex>

#include "../clients/dbus/asus/armoury/intel/pl1_spd_client.hpp"
#include "../clients/dbus/asus/armoury/intel/pl2_sppt_client.hpp"
#include "../clients/dbus/asus/armoury/nvidia/nv_boost_client.hpp"
#include "../clients/dbus/asus/armoury/nvidia/nv_temp_client.hpp"
#include "../clients/dbus/asus/armoury/other/panel_overdrive_client.hpp"
#include "../clients/dbus/asus/core/platform_client.hpp"
#include "../clients/dbus/linux/power_management_kb_brightness.hpp"
#include "../clients/dbus/linux/upower_client.hpp"
#include "../clients/file/boost_control_client.hpp"
#include "../clients/file/cpuinfo_client.hpp"
#include "../clients/file/ssd_scheduler_client.hpp"
#include "../clients/lib/lsusb_client.hpp"
#include "../clients/shell/switcherooctl_client.hpp"
#include "../events/event_bus.hpp"
#include "../gui/toaster.hpp"
#include "../models/hardware/battery_charge_threshold.hpp"
#include "../models/hardware/cpu_brand.hpp"
#include "../models/hardware/usb_identifier.hpp"
#include "../models/performance/ssd_scheduler.hpp"
#include "../services/open_rgb_service.hpp"
#include "../translator/translator.hpp"

class HardwareService : public Singleton<HardwareService>, Loggable {
  public:
	/**
	 * @brief Gets the current battery charge threshold.
	 *
	 * @return The current BatteryThreshold value.
	 */
	BatteryThreshold getChargeThreshold();

	/**
	 * @brief Sets the battery charge threshold.
	 *
	 * @param threshold The new BatteryThreshold value to set.
	 */
	void setChargeThreshold(const BatteryThreshold& threshold);

	/**
	 * @brief Enables or disables panel overdrive.
	 *
	 * @param enabled Set to true to enable panel overdrive, false to disable it.
	 */
	void setPanelOverdrive(const bool& enabled);

	/**
	 * @brief Gets the environment variables required for GPU selection.
	 *
	 * @param gpu The GPU identifier string.
	 * @return A map of environment variable names and their values.
	 */
	std::unordered_map<std::string, std::string> getGpuSelectorEnv(const std::string& gpu);

	/**
	 * @brief Gets the map of available GPUs.
	 *
	 * @return A map where the key is the GPU identifier and the value is its description.
	 */
	std::unordered_map<std::string, std::string> getGpus();

  private:
	friend class Singleton<HardwareService>;
	HardwareService();

	void setupDeviceLoop();
	void onBatteryEvent(const bool& onBattery, const bool& muted = false);
	void onDeviceEvent();

	std::mutex actionMutex;

	EventBus& eventBus						 = EventBus::getInstance();
	Toaster& toaster						 = Toaster::getInstance();
	OpenRgbService& openRgbService			 = OpenRgbService::getInstance();
	Translator& translator					 = Translator::getInstance();
	PlatformClient& platformClient			 = PlatformClient::getInstance();
	Pl1SpdClient& pl1SpdClient				 = Pl1SpdClient::getInstance();
	Pl2SpptClient& pl2SpptClient			 = Pl2SpptClient::getInstance();
	NvBoostClient& nvBoostClient			 = NvBoostClient::getInstance();
	NvTempClient& nvTempClient				 = NvTempClient::getInstance();
	UPowerClient& uPowerClient				 = UPowerClient::getInstance();
	CPUInfoClient& cpuInfoClient			 = CPUInfoClient::getInstance();
	SsdSchedulerClient& ssdSchedulerClient	 = SsdSchedulerClient::getInstance();
	BoostControlClient& boostControlClient	 = BoostControlClient::getInstance();
	SwitcherooCtlClient& switcherooCtlClient = SwitcherooCtlClient::getInstance();
	// PanelOverdriveClient& panelOverdriveClient		 = PanelOverdriveClient::getInstance();
	PMKeyboardBrightness& pmKeyboardBrightnessClient = PMKeyboardBrightness::getInstance();
	LsUsbClient& udevClient							 = LsUsbClient::getInstance();
	Configuration& configuration					 = Configuration::getInstance();

	std::unordered_map<std::string, std::string> gpus;
	std::vector<SsdScheduler> ssd_schedulers;
	std::vector<UsbIdentifier> connectedDevices;

	bool onBattery										= true;
	unsigned int runningGames							= 0;
	BatteryThreshold charge_limit						= BatteryThreshold::Enum::CT_100;
	CpuBrand cpu										= CpuBrand::Enum::INTEL;
	inline static std::vector<std::string> VK_ICD_JSONS = {"/usr/share/vulkan/icd.d/{gpu.value}_icd.json",
														   "/usr/share/vulkan/icd.d/{gpu.value}_icd.i686.json",
														   "/usr/share/vulkan/icd.d/{gpu.value}_icd.x86_64.json"};
};