#pragma once

#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_dbus_client.hpp"

class UPowerClient : public AbstractDbusClient, public Singleton<UPowerClient> {
  public:
	bool isOnBattery();

	void onBatteryChange(CallbackWithAnyParam&& callback);

  private:
	friend class Singleton<UPowerClient>;
	UPowerClient();
};