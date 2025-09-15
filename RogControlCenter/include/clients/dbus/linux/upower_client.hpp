#pragma once

#include "../abstract/abstract_dbus_client.hpp"

class UPowerClient : public AbstractDbusClient {
  public:
	static UPowerClient& getInstance() {
		static UPowerClient instance;
		return instance;
	}

	bool isOnBattery();

	void onBatteryChange(CallbackWithParams&& callback);

  private:
	UPowerClient();
};