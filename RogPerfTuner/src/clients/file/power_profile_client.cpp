#include "clients/file/power_profile_client.hpp"

#include "framework/utils/enum_utils.hpp"

/*PowerProfile PowerProfileClient::getPowerProfile() {
	return fromString<PowerProfile>(this->getProperty<QString>(QString("ActiveProfile")).toStdString(), {{"-", "_"}});
}*/

void PowerProfileClient::setPowerProfile(const PowerProfile& val) {
	write(toString(val));
}

PowerProfileClient::PowerProfileClient() : AbstractFileClient(ACPI_PROFILE_FILE, "PowerProfileClient", true) {
}
