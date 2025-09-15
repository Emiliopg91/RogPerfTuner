
#include "../../../../../include/clients/dbus/asus/core/platform_client.hpp"

BatteryThreshold PlatformClient::getBatteryLimit() {
	return BatteryThreshold::fromInt(this->getProperty<int>(QString("ChargeControlEndThreshold")));
}

void PlatformClient::setBatteryLimit(BatteryThreshold val) {
	this->setProperty<unsigned char>(QString("ChargeControlEndThreshold"), static_cast<unsigned char>(val.toInt()));
}

PlatformProfile PlatformClient::getPlatformProfile() {
	return PlatformProfile::fromInt(this->getProperty<int>(QString("PlatformProfile")));
}

void PlatformClient::setPlatformProfile(PlatformProfile val) {
	this->setProperty<unsigned int>(QString("PlatformProfile"), static_cast<unsigned int>(val.toInt()));
}

bool PlatformClient::getEnablePptGroup() {
	return this->getProperty<bool>(QString("EnablePptGroup"));
}

void PlatformClient::setEnablePptGroup(bool enable) {
	this->setProperty<bool>(QString("EnablePptGroup"), enable);
}

bool PlatformClient::getPlatformProfileLinkedEpp() {
	return this->getProperty<bool>(QString("PlatformProfileLinkedEpp"));
}

void PlatformClient::setPlatformProfileLinkedEpp(bool enable) {
	this->setProperty<bool>(QString("PlatformProfileLinkedEpp"), enable);
}

bool PlatformClient::getChangePlatformProfileOnBattery() {
	return this->getProperty<bool>(QString("ChangePlatformProfileOnBattery"));
}

void PlatformClient::setChangePlatformProfileOnBattery(bool enable) {
	this->setProperty<bool>(QString("ChangePlatformProfileOnBattery"), enable);
}

bool PlatformClient::getChangePlatformProfileOnAc() {
	return this->getProperty<bool>(QString("ChangePlatformProfileOnAc"));
}

void PlatformClient::setChangePlatformProfileOnAc(bool enable) {
	this->setProperty<bool>(QString("ChangePlatformProfileOnAc"), enable);
}

PlatformClient::PlatformClient() : AsusBaseClient("Platform") {
}