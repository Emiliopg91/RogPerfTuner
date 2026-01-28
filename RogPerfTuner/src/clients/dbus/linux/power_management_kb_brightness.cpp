#include "clients/dbus/linux/power_management_kb_brightness.hpp"

int PMKeyboardBrightness::getKeyboardBrightness() {
	return call<int>("keyboardBrightness");
}

void PMKeyboardBrightness::setKeyboardBrightnessSilent(const int& brightness) {
	call("setKeyboardBrightnessSilent", {brightness});
}

void PMKeyboardBrightness::onBrightnessChange(Callback&& callback) {
	this->onSignal("keyboardBrightnessChanged", std::move(callback));
}

PMKeyboardBrightness::PMKeyboardBrightness()
	: AbstractDbusClient(false, "org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement/Actions/KeyboardBrightnessControl",
						 "org.kde.Solid.PowerManagement.Actions.KeyboardBrightnessControl", false) {
}