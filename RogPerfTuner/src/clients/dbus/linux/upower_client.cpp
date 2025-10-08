#include "../../../../include/clients/dbus/linux/upower_client.hpp"

bool UPowerClient::isOnBattery() {
	return this->getProperty<bool>(QString("OnBattery"));
}

void UPowerClient::onBatteryChange(CallbackWithParams&& callback) {
	this->onPropertyChange("OnBattery", std::move(callback));
}

UPowerClient::UPowerClient()
	: AbstractDbusClient(true, QString("org.freedesktop.UPower"), QString("/org/freedesktop/UPower"), QString("org.freedesktop.UPower"), true) {
}