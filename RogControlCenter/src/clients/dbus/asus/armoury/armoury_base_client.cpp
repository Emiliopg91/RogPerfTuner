#include "../../../../../include/clients/dbus/asus/armoury/armoury_base_client.hpp"

ArmouryBaseClient::ArmouryBaseClient(std::string object_path_sufix, bool required)
	: AsusBaseClient("AsusArmoury", "asus_armoury/" + object_path_sufix, required) {
}

int ArmouryBaseClient::getMinValue() {
	return this->getProperty<int>(QString("MinValue"));
}

int ArmouryBaseClient::getMaxValue() {
	return this->getProperty<int>(QString("MaxValue"));
}

int ArmouryBaseClient::getCurrentValue() {
	return this->getProperty<int>(QString("CurrentValue"));
}

void ArmouryBaseClient::setCurrentValue(int value) {
	this->setProperty<int>(QString("CurrentValue"), value);
}