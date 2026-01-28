#pragma once

#include "clients/dbus/abstract/abstract_dbus_client.hpp"
#include "models/others/singleton.hpp"

class UPowerClient : public AbstractDbusClient, public Singleton<UPowerClient> {
  public:
	bool isOnBattery();

	void onBatteryChange(CallbackWithAnyParam&& callback);

  private:
	friend class Singleton<UPowerClient>;
	UPowerClient();
};