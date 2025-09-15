#pragma once

#include "../../../../models/hardware/battery_charge_threshold.hpp"
#include "../../../../models/others/singleton.hpp"
#include "../../../../models/performance/platform_profile.hpp"
#include "../asus_base_client.hpp"

class PlatformClient : public AsusBaseClient, public Singleton<PlatformClient> {
  public:
	BatteryThreshold getBatteryLimit();

	void setBatteryLimit(BatteryThreshold val);

	PlatformProfile getPlatformProfile();

	void setPlatformProfile(PlatformProfile val);

	bool getEnablePptGroup();

	void setEnablePptGroup(bool enable);

	bool getPlatformProfileLinkedEpp();

	void setPlatformProfileLinkedEpp(bool enable);

	bool getChangePlatformProfileOnBattery();

	void setChangePlatformProfileOnBattery(bool enable);

	bool getChangePlatformProfileOnAc();

	void setChangePlatformProfileOnAc(bool enable);

  private:
	friend class Singleton<PlatformClient>;
	PlatformClient();
};