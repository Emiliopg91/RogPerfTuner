#include "utils/event_bus_wrapper.hpp"

EventBusWrapper::EventBusWrapper() : EventBus() {
}

void EventBusWrapper::onDeviceEvent(Callback&& callback) {
	this->on_without_data(UDEV_CLIENT_DEVICE_EVENT, callback);
}
void EventBusWrapper::emitDeviceEvent() {
	this->emit_event(UDEV_CLIENT_DEVICE_EVENT);
}

void EventBusWrapper::onApplicationStop(Callback&& callback) {
	this->on_without_data(APPLICATION_STOP, callback);
}
void EventBusWrapper::emitApplicationStop() {
	this->emit_event(APPLICATION_STOP);
}

void EventBusWrapper::onRgbBrightness(std::function<void(RgbBrightness)>&& callback) {
	this->on_with_data(ORGB_SERVICE_ON_BRIGHTNESS, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<RgbBrightness>(data[0]));
	});
}
void EventBusWrapper::emitRgbBrightness(const RgbBrightness& brightness) {
	this->emit_event(ORGB_SERVICE_ON_BRIGHTNESS, {brightness});
}

void EventBusWrapper::onRgbColor(std::function<void(std::optional<std::string>)>&& callback) {
	this->on_with_data(ORGB_SERVICE_ON_COLOR, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::optional<std::string>>(data[0]));
	});
}
void EventBusWrapper::emitRgbColor(std::optional<std::string> color) {
	this->emit_event(ORGB_SERVICE_ON_COLOR, {color});
}

void EventBusWrapper::onChargeThreshold(std::function<void(BatteryThreshold)>&& callback) {
	this->on_with_data(HARDWARE_SERVICE_THRESHOLD_CHANGED, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<BatteryThreshold>(data[0]));
	});
}
void EventBusWrapper::emitChargeThreshold(const BatteryThreshold& threshold) {
	this->emit_event(HARDWARE_SERVICE_THRESHOLD_CHANGED, {threshold});
}

void EventBusWrapper::onRgbEffect(std::function<void(std::string)>&& callback) {
	this->on_with_data(ORGB_SERVICE_ON_EFFECT, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::string>(data[0]));
	});
}
void EventBusWrapper::emitRgbEffect(const std::string& effect) {
	this->emit_event(ORGB_SERVICE_ON_EFFECT, {effect});
}

void EventBusWrapper::onPerformanceProfile(std::function<void(PerformanceProfile)>&& callback) {
	this->on_with_data(PROFILE_SERVICE_ON_PROFILE, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<PerformanceProfile>(data[0]));
	});
}
void EventBusWrapper::emitPerformanceProfile(const PerformanceProfile& profile) {
	this->emit_event(PROFILE_SERVICE_ON_PROFILE, {profile});
}

void EventBusWrapper::onGameEvent(std::function<void(size_t)>&& callback) {
	this->on_with_data(STEAM_SERVICE_GAME_EVENT, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<size_t>(data[0]));
	});
}
void EventBusWrapper::emitGameEvent(const size_t& runningGames) {
	this->emit_event(STEAM_SERVICE_GAME_EVENT, {runningGames});
}

void EventBusWrapper::onUsbAdded(Callback&& callback) {
	this->on_without_data(HARDWARE_SERVICE_USB_ADDED, callback);
}
void EventBusWrapper::emitUsbAdded() {
	this->emit_event(HARDWARE_SERVICE_USB_ADDED);
}

void EventBusWrapper::onUsbRemoved(std::function<void(std::vector<UsbIdentifier>)>&& callback) {
	this->on_with_data(PROFILE_SERVICE_ON_PROFILE, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::vector<UsbIdentifier>>(data[0]));
	});
}
void EventBusWrapper::emitUsbRemoved(const std::vector<UsbIdentifier>& devices) {
	this->emit_event(HARDWARE_SERVICE_USB_REMOVED, {devices});
}

void EventBusWrapper::onBattery(std::function<void(bool)>&& callback) {
	this->on_with_data(HARDWARE_SERVICE_ON_BATTERY, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<bool>(data[0]));
	});
}
void EventBusWrapper::emitBattery(const bool& onBat) {
	this->emit_event(HARDWARE_SERVICE_ON_BATTERY, {onBat});
}

void EventBusWrapper::onScheduler(std::function<void(std::optional<std::string>)>&& callback) {
	this->on_with_data(PROFILE_SERVICE_ON_SCHEDULER, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::optional<std::string>>(data[0]));
	});
}
void EventBusWrapper::emitScheduler(std::optional<std::string> scheduler) {
	this->emit_event(PROFILE_SERVICE_ON_SCHEDULER, {scheduler});
}

void EventBusWrapper::onBootSound(std::function<void(bool)>&& callback) {
	this->on_with_data(HARDWARE_SERVICE_BOOT_SOUND_CHANGED, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<bool>(data[0]));
	});
}
void EventBusWrapper::emitBootSound(bool value) {
	this->emit_event(HARDWARE_SERVICE_BOOT_SOUND_CHANGED, {value});
}

void EventBusWrapper::emitDbusPropertyEvent(std::string interface, std::string property, std::any value) {
	this->emit_event("dbus." + interface + ".property." + property, {value});
}

void EventBusWrapper::onDbusPropertyEvent(std::string interface, std::string property, CallbackWithAnyParam&& callback) {
	this->on_with_data("dbus." + interface + ".property." + property, [cb = std::move(callback)](CallbackParam data) {
		cb(data[0]);
	});
}

void EventBusWrapper::emitDbusSignalEvent(std::string interface, std::string property) {
	this->emit_event("dbus." + interface + ".signal." + property);
}

void EventBusWrapper::onDbusSignalEvent(std::string interface, std::string property, Callback&& callback) {
	this->on_without_data("dbus." + interface + ".property." + property, [cb = std::move(callback)]() {
		cb();
	});
}

void EventBusWrapper::emitServerSocketEvent(std::string event, CallbackParam value) {
	this->emit_event("socket.server.event." + event, value);
}

void EventBusWrapper::onServerSocketEvent(std::string event, CallbackWithParams&& callback) {
	this->on_with_data("socket.server.event." + event, [cb = std::move(callback)](CallbackParam data) {
		cb(data);
	});
}

void EventBusWrapper::emitUnixSocketEvent(std::string name, std::string event, CallbackParam value) {
	this->emit_event("unix.socket." + name + ".event." + event, value);
}

void EventBusWrapper::onUnixSocketEvent(std::string name, std::string event, CallbackWithParams&& callback) {
	this->on_with_data("unix.socket." + name + ".event." + event, [cb = std::move(callback)](CallbackParam data) {
		cb(data);
	});
}

void EventBusWrapper::emitUpdateAvailable(std::string version) {
	this->emit_event(APPLICATION_SERVICE_UPDATE_AVAILABLE, {version});
}

void EventBusWrapper::onUpdateAvailable(std::function<void(std::string)>&& callback) {
	this->on_with_data(APPLICATION_SERVICE_UPDATE_AVAILABLE, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::string>(data[0]));
	});
}

void EventBusWrapper::emitUpdateStart() {
	this->emit_event(APPLICATION_SERVICE_UPDATE_START);
}

void EventBusWrapper::onUpdateStart(Callback&& callback) {
	this->on_without_data(APPLICATION_SERVICE_UPDATE_START, [cb = std::move(callback)]() {
		cb();
	});
}