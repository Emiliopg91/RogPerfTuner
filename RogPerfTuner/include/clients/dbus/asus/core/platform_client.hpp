#pragma once

#include "../../../../models/hardware/battery_charge_threshold.hpp"
#include "../../../../models/others/singleton.hpp"
#include "../../../../models/performance/platform_profile.hpp"
#include "../asus_base_client.hpp"

class PlatformClient : public AsusBaseClient, public Singleton<PlatformClient> {
  public:
	/**
	 * @brief Retrieves the current battery charge threshold limit.
	 *
	 * This function returns the configured battery threshold, which determines
	 * the maximum charge level for the battery. This can be used to extend
	 * battery lifespan by preventing full charge cycles.
	 *
	 * @return BatteryThreshold The current battery charge limit setting.
	 */
	BatteryThreshold getBatteryLimit();

	/**
	 * @brief Sets the battery charge limit to the specified threshold.
	 *
	 * This function configures the maximum battery charge level, which can help
	 * prolong battery lifespan by preventing it from charging to 100% all the time.
	 *
	 * @param val The desired battery threshold value to set.
	 */
	void setBatteryLimit(BatteryThreshold val);

	/**
	 * @brief Retrieves the current platform profile.
	 *
	 * This function queries the underlying system or hardware to obtain the
	 * current platform profile, which may represent different performance,
	 * power, or thermal configurations (such as "Silent", "Performance", or "Turbo").
	 *
	 * @return PlatformProfile The current platform profile setting.
	 */
	PlatformProfile getPlatformProfile();

	/**
	 * @brief Sets the current platform profile.
	 *
	 * This method updates the platform profile to the specified value.
	 * The platform profile typically controls system performance, power,
	 * or thermal characteristics depending on the selected profile.
	 *
	 * @param val The new platform profile to set.
	 */
	void setPlatformProfile(PlatformProfile val);

	/**
	 * @brief Retrieves the current status of the PPT (Platform Power Target) group enablement.
	 *
	 * @return true if the PPT group is enabled, false otherwise.
	 */
	bool getEnablePptGroup();

	/**
	 * @brief Enables or disables the PPT (Platform Power Target) group.
	 *
	 * This function allows the user to enable or disable the PPT group, which may control
	 * power management features or performance tuning on supported ASUS platforms.
	 *
	 * @param enable Set to true to enable the PPT group, or false to disable it.
	 */
	void setEnablePptGroup(bool enable);

	/**
	 * @brief Retrieves the status of the platform profile's EPP (Energy Performance Preference) linkage.
	 *
	 * This function checks whether the platform profile is currently linked to the EPP setting,
	 * which may affect system performance and power management behavior.
	 *
	 * @return true if the platform profile is linked to EPP, false otherwise.
	 */
	bool getPlatformProfileLinkedEpp();

	/**
	 * @brief Enables or disables the linkage between the platform profile and EPP (Energy Performance Preference).
	 *
	 * When enabled, the platform profile will be linked to the EPP setting, allowing coordinated power and performance management.
	 *
	 * @param enable Set to true to enable the linkage, or false to disable it.
	 */
	void setPlatformProfileLinkedEpp(bool enable);

	/**
	 * @brief Retrieves the current setting for changing the platform profile when running on battery.
	 *
	 * This function checks whether the platform profile should automatically change
	 * when the device is operating on battery power.
	 *
	 * @return true if the platform profile changes on battery, false otherwise.
	 */
	bool getChangePlatformProfileOnBattery();

	/**
	 * @brief Enables or disables changing the platform profile when running on battery power.
	 *
	 * @param enable Set to true to allow changing the platform profile on battery, false to disable.
	 */
	void setChangePlatformProfileOnBattery(bool enable);

	/**
	 * @brief Retrieves the current setting for changing the platform profile when on AC power.
	 *
	 * This function checks whether the platform profile should automatically change
	 * when the device is connected to AC power.
	 *
	 * @return true if the platform profile changes on AC power; false otherwise.
	 */
	bool getChangePlatformProfileOnAc();

	/**
	 * @brief Enables or disables changing the platform profile when on AC power.
	 *
	 * When enabled, the platform profile will automatically change based on the AC power state.
	 *
	 * @param enable Set to true to enable automatic platform profile change on AC power, false to disable.
	 */
	void setChangePlatformProfileOnAc(bool enable);

  private:
	friend class Singleton<PlatformClient>;
	PlatformClient();
};