#pragma once

#include "../../../../models/performance/platform_profile.hpp"
#include "../asus_base_client.hpp"
#include "RccCommons.hpp"

class FanCurvesClient : public AsusBaseClient {
  public:
	static FanCurvesClient& getInstance() {
		static FanCurvesClient instance;
		return instance;
	}

	void resetProfileCurve(PlatformProfile profile) {
		call("ResetPowerProfile", {static_cast<uint>(profile.toInt())});
	}

	void setCurveToDefaults(PlatformProfile profile) {
		call("SetCurvesToDefaults", {static_cast<uint>(profile.toInt())});
	}

	void setFanCurveEnabled(PlatformProfile profile) {
		call("SetFanCurvesEnabled", {static_cast<uint>(profile.toInt()), true});
	}

  private:
	FanCurvesClient() : AsusBaseClient("FanCurves") {
	}
};