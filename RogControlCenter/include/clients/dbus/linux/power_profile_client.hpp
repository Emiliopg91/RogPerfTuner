#pragma once

#include "../../../models/performance/power_profile.hpp"
#include "../abstract/abstract_dbus_client.hpp"

class PowerProfileClient : public AbstractDbusClient {
  public:
	static PowerProfileClient& getInstance() {
		static PowerProfileClient instance;
		return instance;
	}

	const PowerProfile getPowerProfile();

	void setPowerProfile(const PowerProfile& val);

  private:
	PowerProfileClient();
};