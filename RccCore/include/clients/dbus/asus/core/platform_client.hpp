#pragma once

#include "../../../../models/hardware/battery_charge_threshold.hpp"
#include "../../../../models/performance/platform_profile.hpp"
#include "../asus_base_client.hpp"

class PlatformClient : public AsusBaseClient {
  public:
	static PlatformClient& getInstance() {
		static PlatformClient instance;
		return instance;
	}

	BatteryThreshold getBatteryLimit() {
		return BatteryThreshold::fromInt(this->getProperty<int>(QString("ChargeControlEndThreshold")));
	}

	void setBatteryLimit(BatteryThreshold val) {
		this->setProperty<unsigned char>(QString("ChargeControlEndThreshold"), static_cast<unsigned char>(val.toInt()));
	}

	PlatformProfile getPlatformProfile() {
		return PlatformProfile::fromInt(this->getProperty<int>(QString("PlatformProfile")));
	}

	void setPlatformProfile(PlatformProfile val) {
		this->setProperty<unsigned int>(QString("PlatformProfile"), static_cast<unsigned int>(val.toInt()));
	}

	bool getEnablePptGroup() {
		return this->getProperty<bool>(QString("EnablePptGroup"));
	}

	void setEnablePptGroup(bool enable) {
		this->setProperty<bool>(QString("EnablePptGroup"), enable);
	}

	bool getPlatformProfileLinkedEpp() {
		return this->getProperty<bool>(QString("PlatformProfileLinkedEpp"));
	}

	void setPlatformProfileLinkedEpp(bool enable) {
		this->setProperty<bool>(QString("PlatformProfileLinkedEpp"), enable);
	}

	bool getChangePlatformProfileOnBattery() {
		return this->getProperty<bool>(QString("ChangePlatformProfileOnBattery"));
	}

	void setChangePlatformProfileOnBattery(bool enable) {
		this->setProperty<bool>(QString("ChangePlatformProfileOnBattery"), enable);
	}

	bool getChangePlatformProfileOnAc() {
		return this->getProperty<bool>(QString("ChangePlatformProfileOnAc"));
	}

	void setChangePlatformProfileOnAc(bool enable) {
		this->setProperty<bool>(QString("ChangePlatformProfileOnAc"), enable);
	}

  private:
	PlatformClient() : AsusBaseClient("Platform") {
	}
};