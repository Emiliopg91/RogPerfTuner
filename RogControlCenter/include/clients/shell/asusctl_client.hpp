#pragma once

#include "../../models/hardware/fan_curve_data.hpp"
#include "../../models/others/singleton.hpp"
#include "../../models/performance/platform_profile.hpp"
#include "./abstract/abstract_cmd_client.hpp"

class AsusCtlClient : public AbstractCmdClient, public Singleton<AsusCtlClient> {
  private:
	AsusCtlClient() : AbstractCmdClient("asusctl", "AsusCtlClient") {
	}
	friend class Singleton<AsusCtlClient>;

  public:
	void turnOffAura();

	void setCurvesToDefaults(PlatformProfile profile);

	void setFanCurvesEnabled(PlatformProfile profile, bool enabled);

	void setFanCurveData(PlatformProfile profile, std::string fanName, FanCurveData data);
	void setFanCurveStringData(PlatformProfile profile, std::string fanName, std::string data);

	std::unordered_map<std::string, FanCurveData> getFanCurveData(PlatformProfile profile);
};