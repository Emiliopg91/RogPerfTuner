#include "../../../../include/clients/dbus/linux/power_profile_client.hpp"

PowerProfile PowerProfileClient::getPowerProfile() {
	return PowerProfileNS::fromString(this->getProperty<QString>(QString("ActiveProfile")).toStdString());
}

void PowerProfileClient::setPowerProfile(const PowerProfile& val) {
	this->setProperty<QString>(QString("ActiveProfile"), QString::fromStdString(PowerProfileNS::toString(val)));
}

PowerProfileClient::PowerProfileClient()
	: AbstractDbusClient(true, QString("net.hadess.PowerProfiles"), QString("/net/hadess/PowerProfiles"), QString("net.hadess.PowerProfiles"), true) {
}
