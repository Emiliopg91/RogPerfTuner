#include "../../include/events/event_bus.hpp"

#include <any>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

void EventBus::on_without_data(const std::string& event, Callback callback) {
	std::lock_guard<std::mutex> lock(mtx);
	no_params_listeners[event].emplace_back(std::move(callback));
}

void EventBus::on_without_data(const std::string_view& event, Callback callback) {
	on_without_data(std::string(event), std::move(callback));
}

void EventBus::emit_event(const std::string_view& event) {
	emit_event(std::string(event));
}

void EventBus::emit_event(const std::string& event) {
	std::vector<std::function<void()>> to_call;
	{
		std::lock_guard<std::mutex> lock(mtx);
		auto it = no_params_listeners.find(event);
		if (it == no_params_listeners.end()) {
			return;
		}
		to_call = it->second;
	}

	for (auto& callback : to_call) {
		try {
			callback();
		} catch (const std::exception& e) {
		}
	}
}

void EventBus::on_with_data(const std::string_view& event, CallbackWithParams&& callback) {
	on_with_data(std::string(event), std::move(callback));
}

void EventBus::on_with_data(const std::string& event, CallbackWithParams&& callback) {
	std::lock_guard<std::mutex> lock(mtx);
	auto it = with_params_listeners.find(event);

	if (it == with_params_listeners.end()) {
		auto holder = std::vector<CallbackWithParams>();
		holder.push_back(std::move(callback));
		with_params_listeners[event] = std::move(holder);
	} else {
		with_params_listeners[event].push_back(std::move(callback));
	}
}

void EventBus::emit_event(const std::string_view& event, const std::vector<std::any>& args) {
	emit_event(std::string(event), args);
}

void EventBus::emit_event(const std::string& event, const std::vector<std::any>& args) {
	std::vector<std::function<void(std::vector<std::any>)>> to_call;

	{
		std::lock_guard<std::mutex> lock(mtx);
		auto it = with_params_listeners.find(event);
		if (it == with_params_listeners.end()) {
			return;
		}

		to_call = it->second;
	}

	for (auto& callback : to_call) {
		try {
			callback(args);
		} catch (const std::exception& e) {
		}
	}
}

void EventBus::onDeviceEvent(Callback&& callback) {
	this->on_without_data(UDEV_CLIENT_DEVICE_EVENT, callback);
}
void EventBus::emitDeviceEvent() {
	this->emit_event(UDEV_CLIENT_DEVICE_EVENT);
}

void EventBus::onApplicationStop(Callback&& callback) {
	this->on_without_data(APPLICATION_STOP, callback);
}
void EventBus::emitApplicationStop() {
	this->emit_event(APPLICATION_STOP);
}

void EventBus::onRgbBrightness(std::function<void(RgbBrightness)>&& callback) {
	this->on_with_data(ORGB_SERVICE_ON_BRIGHTNESS, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<RgbBrightness>(data[0]));
	});
}
void EventBus::emitRgbBrightness(const RgbBrightness& brightness) {
	this->emit_event(ORGB_SERVICE_ON_BRIGHTNESS, {brightness});
}

void EventBus::onChargeThreshold(std::function<void(BatteryThreshold)>&& callback) {
	this->on_with_data(HARDWARE_SERVICE_THRESHOLD_CHANGED, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<BatteryThreshold>(data[0]));
	});
}
void EventBus::emitChargeThreshold(const BatteryThreshold& threshold) {
	this->emit_event(HARDWARE_SERVICE_THRESHOLD_CHANGED, {threshold});
}

void EventBus::onRgbEffect(std::function<void(std::string)>&& callback) {
	this->on_with_data(ORGB_SERVICE_ON_EFFECT, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::string>(data[0]));
	});
}
void EventBus::emitRgbEffect(const std::string& effect) {
	this->emit_event(ORGB_SERVICE_ON_EFFECT, {effect});
}

void EventBus::onPerformanceProfile(std::function<void(PerformanceProfile)>&& callback) {
	this->on_with_data(PROFILE_SERVICE_ON_PROFILE, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<PerformanceProfile>(data[0]));
	});
}
void EventBus::emitPerformanceProfile(const PerformanceProfile& profile) {
	this->emit_event(PROFILE_SERVICE_ON_PROFILE, {profile});
}

void EventBus::onGameEvent(std::function<void(size_t)>&& callback) {
	this->on_with_data(STEAM_SERVICE_GAME_EVENT, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<size_t>(data[0]));
	});
}
void EventBus::emitGameEvent(const size_t& runningGames) {
	this->emit_event(STEAM_SERVICE_GAME_EVENT, {runningGames});
}

void EventBus::onUsbAdded(Callback&& callback) {
	this->on_without_data(HARDWARE_SERVICE_USB_ADDED, callback);
}
void EventBus::emitUsbAdded() {
	this->emit_event(HARDWARE_SERVICE_USB_ADDED);
}

void EventBus::onUsbRemoved(std::function<void(std::vector<UsbIdentifier>)>&& callback) {
	this->on_with_data(PROFILE_SERVICE_ON_PROFILE, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<std::vector<UsbIdentifier>>(data[0]));
	});
}
void EventBus::emitUsbRemoved(const std::vector<UsbIdentifier>& devices) {
	this->emit_event(HARDWARE_SERVICE_USB_REMOVED, {devices});
}

void EventBus::onBattery(std::function<void(bool)>&& callback) {
	this->on_with_data(HARDWARE_SERVICE_ON_BATTERY, [cb = std::move(callback)](CallbackParam data) {
		cb(std::any_cast<bool>(data[0]));
	});
}
void EventBus::emitBattery(const bool& onBat) {
	this->emit_event(HARDWARE_SERVICE_ON_BATTERY, {onBat});
}