#pragma once

#include "abstracts/singleton.hpp"
#include "clients/abstract_dbus_client.hpp"
#include "models/performance/power_profile.hpp"

class PowerProfileClient : public AbstractDbusClient, public Singleton<PowerProfileClient> {
  public:
	PowerProfile getPowerProfile();

	void setPowerProfile(const PowerProfile& val);

  private:
	friend class Singleton<PowerProfileClient>;
	PowerProfileClient();
};