#pragma once

#include "../abstract/abstract_dbus_client.hpp"
#include "RccCommons.hpp"

class UPowerClient : public AbstractDbusClient {
  public:
	static UPowerClient& getInstance() {
		static UPowerClient instance;
		return instance;
	}

	bool isOnBattery() {
		return this->getProperty<bool>(QString("OnBattery"));
	}

	void onBatteryChange(CallbackWithParams&& callback) {
		this->onPropertyChange("OnBattery", std::move(callback));
	}

  private:
	UPowerClient()
		: AbstractDbusClient(true, QString("org.freedesktop.UPower"), QString("/org/freedesktop/UPower"), QString("org.freedesktop.UPower"), true) {
	}
};