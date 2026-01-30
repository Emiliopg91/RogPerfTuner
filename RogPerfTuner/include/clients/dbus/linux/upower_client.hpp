#pragma once

#include "abstracts/clients/abstract_dbus_client.hpp"
#include "abstracts/singleton.hpp"

class UPowerClient : public AbstractDbusClient, public Singleton<UPowerClient> {
  public:
	bool isOnBattery();

	void onBatteryChange(CallbackWithAnyParam&& callback);

  private:
	friend class Singleton<UPowerClient>;
	UPowerClient();
};