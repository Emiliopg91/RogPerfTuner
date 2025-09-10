#pragma once

#include "../../../models/performance/power_profile.hpp"
#include "../abstract/abstract_dbus_client.hpp"

class PowerProfileClient : public AbstractDbusClient {
  public:
	static PowerProfileClient& getInstance() {
		static PowerProfileClient instance;
		return instance;
	}

	const PowerProfile getPowerProfile() {
		return PowerProfile::fromString(this->getProperty<QString>(QString("ActiveProfile")).toStdString());
	}

	void setPowerProfile(const PowerProfile& val) {
		this->setProperty<QString>(QString("ActiveProfile"), QString::fromStdString(val.toString()));
	}

  private:
	PowerProfileClient()
		: AbstractDbusClient(true, QString("net.hadess.PowerProfiles"), QString("/net/hadess/PowerProfiles"), QString("net.hadess.PowerProfiles"),
							 true) {
	}
};