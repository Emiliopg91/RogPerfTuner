#pragma once

#include "../../../../models/performance/platform_profile.hpp"
#include "../asus_base_client.hpp"

class FanCurvesClient : public AsusBaseClient {
  public:
	static FanCurvesClient& getInstance() {
		static FanCurvesClient instance;
		return instance;
	}

	void resetProfileCurve(PlatformProfile profile);

	void setCurveToDefaults(PlatformProfile profile);

	void setFanCurveEnabled(PlatformProfile profile);

  private:
	FanCurvesClient();
};