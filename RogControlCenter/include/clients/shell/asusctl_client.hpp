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

	void setCurveToDefaults();

	void setFanCurvesEnabled(PlatformProfile profile);

	void setFanCurveData(PlatformProfile profile, std::string fanName, FanCurveData data);

	std::unordered_map<std::string, FanCurveData> getFanCurveData(PlatformProfile profile);
};