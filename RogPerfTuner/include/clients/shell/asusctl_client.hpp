#pragma once

#include "abstracts/clients/abstract_cmd_client.hpp"
#include "abstracts/singleton.hpp"
#include "models/hardware/fan_curve_data.hpp"
#include "models/performance/platform_profile.hpp"

class AsusCtlClient : public AbstractCmdClient, public Singleton<AsusCtlClient> {
  private:
	AsusCtlClient() : AbstractCmdClient("asusctl", "AsusCtlClient") {
	}
	friend class Singleton<AsusCtlClient>;

  public:
	/**
	 * @brief Turns off the Aura lighting on the device.
	 *
	 * This function disables or powers down the Aura RGB lighting system,
	 * ensuring that all associated lights are turned off.
	 */
	void turnOffAura();

	/**
	 * @brief Resets the fan or performance curves to their default values for the specified platform profile.
	 *
	 * This function restores the default curve settings associated with the provided PlatformProfile.
	 *
	 * @param profile The platform profile for which to reset the curves to default values.
	 */
	void setCurvesToDefaults(PlatformProfile profile);

	/**
	 * @brief Enables or disables custom fan curves for the specified platform profile.
	 *
	 * This function allows toggling the use of custom fan curves for a given
	 * hardware or performance profile. When enabled, the system will use
	 * user-defined fan curves instead of default settings.
	 *
	 * @param profile The platform profile for which to set the fan curve state.
	 * @param enabled Set to true to enable custom fan curves, or false to disable them.
	 */
	void setFanCurvesEnabled(PlatformProfile profile, bool enabled);

	/**
	 * @brief Sets the fan curve data for a specific fan and platform profile.
	 *
	 * This function applies the provided fan curve configuration to the specified fan
	 * under the given platform profile. The fan curve data determines how the fan speed
	 * responds to temperature changes.
	 *
	 * @param profile The platform profile to which the fan curve will be applied.
	 * @param fanName The name or identifier of the fan to configure.
	 * @param data The fan curve data containing the speed/temperature mapping.
	 */
	void setFanCurveData(PlatformProfile profile, std::string fanName, FanCurveData data);

	/**
	 * @brief Sets the fan curve data for a specific fan and platform profile.
	 *
	 * This function applies the provided fan curve configuration to the specified fan
	 * under the given platform profile. The fan curve data determines how the fan speed
	 * responds to temperature changes.
	 *
	 * @param profile The platform profile to which the fan curve will be applied.
	 * @param fanName The name or identifier of the fan to configure.
	 * @param data The fan curve data string.
	 */
	void setFanCurveStringData(PlatformProfile profile, std::string fanName, std::string data);

	/**
	 * @brief Retrieves the fan curve data for the specified platform profile.
	 *
	 * This function queries and returns a mapping of fan identifiers to their corresponding
	 * fan curve data, based on the provided platform profile.
	 *
	 * @param profile The platform profile for which to retrieve fan curve data.
	 * @return std::unordered_map<std::string, FanCurveData>
	 *         A map where the key is the fan identifier (as a string) and the value is the associated FanCurveData.
	 */
	std::unordered_map<std::string, FanCurveData> getFanCurveData(PlatformProfile profile);
};