#pragma once

#include <mutex>

#include "clients/dbus/asus/core/platform_client.hpp"
#ifdef BAT_LIMIT
#include "clients/file/battery_charge_limit_client.hpp"
#endif
#ifdef BAT_STATUS
#include "clients/file/battery_status_client.hpp"
#endif
#ifdef BOOT_SOUND
#include "clients/file/firmware/asus-armoury/other/boot_sound_client.hpp"
#endif
#ifdef PANEL_OD
#include "clients/file/firmware/asus-armoury/other/panel_overdrive_client.hpp"
#endif
#include "clients/lib/udev_client.hpp"
#include "framework/gui/toaster.hpp"
#include "framework/translator/translator.hpp"
#include "models/hardware/battery_charge_threshold.hpp"
#include "models/hardware/usb_identifier.hpp"
#include "services/open_rgb_service.hpp"
#include "utils/event_bus_wrapper.hpp"

class HardwareService : public Singleton<HardwareService>, Loggable {
  public:
#ifdef BAT_LIMIT
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
#endif

#ifdef BOOT_SOUND
	/**
	 * @brief Gets the current boot sound status.
	 *
	 * @return The current boot sound value.
	 */
	bool getBootSound();

	/**
	 * @brief Sets the boot sound flag.
	 *
	 * @param threshold The new boot sound value to set.
	 */
	void setBootSound(bool enable);
#endif

#ifdef PANEL_OD
	/**
	 * @brief Enables or disables panel overdrive.
	 *
	 * @param enabled Set to true to enable panel overdrive, false to disable it.
	 */
	void setPanelOverdrive(bool enabled);
#endif

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
	void onBatteryEvent(bool onBattery, bool muted = false);
	void onDeviceEvent();

	std::mutex actionMutex;

	EventBusWrapper& eventBus	   = EventBusWrapper::getInstance();
	Toaster& toaster			   = Toaster::getInstance();
	OpenRgbService& openRgbService = OpenRgbService::getInstance();
	Translator& translator		   = Translator::getInstance();
#ifdef BAT_LIMIT
	BatteryChargeLimitClient& batteryChargeLimitClient = BatteryChargeLimitClient::getInstance();
#endif
	PlatformClient& platformClient = PlatformClient::getInstance();
#ifdef BOOT_SOUND
	BootSoundClient& bootSoundClient = BootSoundClient::getInstance();
#endif
#ifdef BAT_STATUS
	BatteryStatusClient& batteryStatusClient = BatteryStatusClient::getInstance();
#endif
#ifdef PANEL_OD
	PanelOverdriveClient& panelOverdriveClient = PanelOverdriveClient::getInstance();
#endif
	UdevClient& udevClient				= UdevClient::getInstance();
	ConfigurationWrapper& configuration = ConfigurationWrapper::getInstance();

	std::unordered_map<std::string, std::string> gpus;
	std::vector<UsbIdentifier> connectedDevices;

	bool onBattery				  = true;
	unsigned int runningGames	  = 0;
	BatteryThreshold charge_limit = BatteryThreshold::CT_100;
};