#pragma once

#include "../../../../models/others/singleton.hpp"
#include "../../../../models/performance/platform_profile.hpp"
#include "../asus_base_client.hpp"

class FanCurvesClient : public AsusBaseClient, public Singleton<FanCurvesClient> {
  public:
	void resetProfileCurve(PlatformProfile profile);

	void setCurveToDefaults(PlatformProfile profile);

	void setFanCurveEnabled(PlatformProfile profile);

  private:
	FanCurvesClient();
	friend class Singleton<FanCurvesClient>;
};