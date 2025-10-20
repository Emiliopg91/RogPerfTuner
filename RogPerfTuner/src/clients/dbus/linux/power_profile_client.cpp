#include "../../../../include/clients/dbus/linux/power_profile_client.hpp"

const PowerProfile PowerProfileClient::getPowerProfile() {
	return PowerProfile::fromString(this->getProperty<QString>(QString("ActiveProfile")).toStdString());
}

void PowerProfileClient::setPowerProfile(const PowerProfile& val) {
	this->setProperty<QString>(QString("ActiveProfile"), QString::fromStdString(val.toString()));
}

PowerProfileClient::PowerProfileClient()
	: AbstractDbusClient(true, QString("net.hadess.PowerProfiles"), QString("/net/hadess/PowerProfiles"), QString("net.hadess.PowerProfiles"), true) {
}
