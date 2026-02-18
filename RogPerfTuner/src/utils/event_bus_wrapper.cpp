#include "utils/event_bus_wrapper.hpp"

#include "framework/utils/enum_utils.hpp"
#include "models/others/events.hpp"

EventBusWrapper::EventBusWrapper() {
}

void EventBusWrapper::onDeviceEvent(Callback&& callback) {
	this->eventBus.on_without_data(toName(Events::UDEV_CLIENT_DEVICE_EVENT), callback);
}
void EventBusWrapper::emitDeviceEvent() {
	this->eventBus.emit_event(toName(Events::UDEV_CLIENT_DEVICE_EVENT));
}

void EventBusWrapper::onApplicationShutdown(Callback&& callback) {
	this->eventBus.on_without_data(toName(Events::APPLICATION_SHUTDOWN), callback);
}
void EventBusWrapper::emitApplicationShutdown() {
	this->eventBus.emit_event(toName(Events::APPLICATION_SHUTDOWN));
}

void EventBusWrapper::onApplicationStop(Callback&& callback) {
	this->eventBus.on_without_data(toName(Events::APPLICATION_STOP), callback);
}
void EventBusWrapper::emitApplicationStop() {
	this->eventBus.emit_event(toName(Events::APPLICATION_STOP));
}

void EventBusWrapper::onRgbBrightness(std::function<void(RgbBrightness)>&& callback) {
	this->eventBus.on_with_data(toName(Events::ORGB_SERVICE_ON_BRIGHTNESS), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<RgbBrightness>(data[0]));
	});
}
void EventBusWrapper::emitRgbBrightness(const RgbBrightness& brightness) {
	this->eventBus.emit_event(toName(Events::ORGB_SERVICE_ON_BRIGHTNESS), {brightness});
}

void EventBusWrapper::onRgbColor(std::function<void(std::optional<std::string>)>&& callback) {
	this->eventBus.on_with_data(toName(Events::ORGB_SERVICE_ON_COLOR), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::optional<std::string>>(data[0]));
	});
}
void EventBusWrapper::emitRgbColor(std::optional<std::string> color) {
	this->eventBus.emit_event(toName(Events::ORGB_SERVICE_ON_COLOR), {color});
}

void EventBusWrapper::onChargeThreshold(std::function<void(BatteryThreshold)>&& callback) {
	this->eventBus.on_with_data(toName(Events::HARDWARE_SERVICE_THRESHOLD_CHANGED), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<BatteryThreshold>(data[0]));
	});
}
void EventBusWrapper::emitChargeThreshold(const BatteryThreshold& threshold) {
	this->eventBus.emit_event(toName(Events::HARDWARE_SERVICE_THRESHOLD_CHANGED), {threshold});
}

void EventBusWrapper::onRgbEffect(std::function<void(std::string)>&& callback) {
	this->eventBus.on_with_data(toName(Events::ORGB_SERVICE_ON_EFFECT), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::string>(data[0]));
	});
}
void EventBusWrapper::emitRgbEffect(const std::string& effect) {
	this->eventBus.emit_event(toName(Events::ORGB_SERVICE_ON_EFFECT), {effect});
}

void EventBusWrapper::onPerformanceProfile(std::function<void(PerformanceProfile)>&& callback) {
	this->eventBus.on_with_data(toName(Events::PROFILE_SERVICE_ON_PROFILE), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<PerformanceProfile>(data[0]));
	});
}
void EventBusWrapper::emitPerformanceProfile(const PerformanceProfile& profile) {
	this->eventBus.emit_event(toName(Events::PROFILE_SERVICE_ON_PROFILE), {profile});
}

void EventBusWrapper::onGameEvent(std::function<void(size_t)>&& callback) {
	this->eventBus.on_with_data(toName(Events::STEAM_SERVICE_GAME_EVENT), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<size_t>(data[0]));
	});
}
void EventBusWrapper::emitGameEvent(const size_t& runningGames) {
	this->eventBus.emit_event(toName(Events::STEAM_SERVICE_GAME_EVENT), {runningGames});
}

void EventBusWrapper::onUsbAdded(Callback&& callback) {
	this->eventBus.on_without_data(toName(Events::HARDWARE_SERVICE_USB_ADDED), callback);
}
void EventBusWrapper::emitUsbAdded() {
	this->eventBus.emit_event(toName(Events::HARDWARE_SERVICE_USB_ADDED));
}

void EventBusWrapper::onUsbRemoved(std::function<void(std::vector<UsbIdentifier>)>&& callback) {
	this->eventBus.on_with_data(toName(Events::PROFILE_SERVICE_ON_PROFILE), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::vector<UsbIdentifier>>(data[0]));
	});
}
void EventBusWrapper::emitUsbRemoved(const std::vector<UsbIdentifier>& devices) {
	this->eventBus.emit_event(toName(Events::HARDWARE_SERVICE_USB_REMOVED), {devices});
}

void EventBusWrapper::onBattery(std::function<void(bool)>&& callback) {
	this->eventBus.on_with_data(toName(Events::HARDWARE_SERVICE_ON_BATTERY), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<bool>(data[0]));
	});
}
void EventBusWrapper::emitBattery(const bool& onBat) {
	this->eventBus.emit_event(toName(Events::HARDWARE_SERVICE_ON_BATTERY), {onBat});
}

void EventBusWrapper::onScheduler(std::function<void(std::optional<std::string>)>&& callback) {
	this->eventBus.on_with_data(toName(Events::PROFILE_SERVICE_ON_SCHEDULER), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::optional<std::string>>(data[0]));
	});
}
void EventBusWrapper::emitScheduler(std::optional<std::string> scheduler) {
	this->eventBus.emit_event(toName(Events::PROFILE_SERVICE_ON_SCHEDULER), {scheduler});
}

void EventBusWrapper::onSsdScheduler(std::function<void(std::string)>&& callback) {
	this->eventBus.on_with_data(toName(Events::PROFILE_SERVICE_ON_SSD_SCHEDULER), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::string>(data[0]));
	});
}
void EventBusWrapper::emitSsdScheduler(std::string scheduler) {
	this->eventBus.emit_event(toName(Events::PROFILE_SERVICE_ON_SSD_SCHEDULER), {scheduler});
}

void EventBusWrapper::onBootSound(std::function<void(bool)>&& callback) {
	this->eventBus.on_with_data(toName(Events::HARDWARE_SERVICE_BOOT_SOUND_CHANGED), [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<bool>(data[0]));
	});
}
void EventBusWrapper::emitBootSound(bool value) {
	this->eventBus.emit_event(toName(Events::HARDWARE_SERVICE_BOOT_SOUND_CHANGED), {value});
}

void EventBusWrapper::emitServerSocketEvent(std::string event, CallbackParam value) {
	this->eventBus.emit_event("socket.server.event." + event, value);
}

void EventBusWrapper::onServerSocketEvent(std::string event, CallbackWithParams&& callback) {
	this->eventBus.on_with_data("socket.server.event." + event, [cb = std::move(callback)](CallbackParam data) {
		cb(data);
	});
}