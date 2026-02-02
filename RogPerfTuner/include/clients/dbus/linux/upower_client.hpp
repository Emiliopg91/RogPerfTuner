#pragma once

#include "abstracts/singleton.hpp"
#include "clients/abstract_dbus_client.hpp"

class UPowerClient : public AbstractDbusClient, public Singleton<UPowerClient> {
  public:
	bool isOnBattery();

	void onBatteryChange(CallbackWithAnyParam&& callback);

  private:
	friend class Singleton<UPowerClient>;
	UPowerClient();
};