#pragma once

#include "../../../models/others/singleton.hpp"
#include "../../../models/performance/power_profile.hpp"
#include "../abstract/abstract_dbus_client.hpp"

class PowerProfileClient : public AbstractDbusClient, public Singleton<PowerProfileClient> {
  public:
	PowerProfile getPowerProfile();

	void setPowerProfile(const PowerProfile& val);

  private:
	friend class Singleton<PowerProfileClient>;
	PowerProfileClient();
};