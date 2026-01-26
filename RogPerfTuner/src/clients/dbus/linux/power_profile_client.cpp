#include "../../../../include/clients/dbus/linux/power_profile_client.hpp"

#include "../../../../include/utils/enum_utils.hpp"

PowerProfile PowerProfileClient::getPowerProfile() {
	return fromString<PowerProfile>(this->getProperty<QString>(QString("ActiveProfile")).toStdString(), {{"-", "_"}});
}

void PowerProfileClient::setPowerProfile(const PowerProfile& val) {
	this->setProperty<QString>(QString("ActiveProfile"), QString::fromStdString(toString(val, {{"_", "-"}})));
}

PowerProfileClient::PowerProfileClient()
	: AbstractDbusClient(true, QString("net.hadess.PowerProfiles"), QString("/net/hadess/PowerProfiles"), QString("net.hadess.PowerProfiles"), true) {
}
