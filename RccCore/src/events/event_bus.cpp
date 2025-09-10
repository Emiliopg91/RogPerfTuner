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

void EventBus::emit_event(const std::string& event) {
	std::vector<std::function<void()>> to_call;
	{
		std::lock_guard<std::mutex> lock(mtx);
		auto it = no_params_listeners.find(event);
		if (it == no_params_listeners.end())
			return;
		to_call = it->second;
	}

	for (auto& callback : to_call) {
		try {
			callback();
		} catch (const std::exception& e) {
		}
	}
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

void EventBus::emit_event(const std::string& event, const std::vector<std::any>& args) {
	std::vector<std::function<void(std::vector<std::any>)>> to_call;

	{
		std::lock_guard<std::mutex> lock(mtx);
		auto it = with_params_listeners.find(event);
		if (it == with_params_listeners.end())
			return;

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

void EventBus::onRgbBrightness(CallbackWithParams&& callback) {
	this->on_with_data(ORGB_SERVICE_ON_BRIGHTNESS, std::move(callback));
}
void EventBus::emitRgbBrightness(RgbBrightness brightness) {
	this->emit_event(ORGB_SERVICE_ON_BRIGHTNESS, {brightness});
}

void EventBus::onChargeThreshold(CallbackWithParams&& callback) {
	this->on_with_data(HARDWARE_SERVICE_THRESHOLD_CHANGED, std::move(callback));
}
void EventBus::emitChargeThreshold(BatteryThreshold threshold) {
	this->emit_event(HARDWARE_SERVICE_THRESHOLD_CHANGED, {threshold});
}

void EventBus::onRgbEffect(CallbackWithParams&& callback) {
	this->on_with_data(ORGB_SERVICE_ON_EFFECT, std::move(callback));
}
void EventBus::emitRgbEffect(std::string effect) {
	this->emit_event(ORGB_SERVICE_ON_EFFECT, {effect});
}

void EventBus::onPerformanceProfile(CallbackWithParams&& callback) {
	this->on_with_data(PROFILE_SERVICE_ON_PROFILE, std::move(callback));
}
void EventBus::emitPerformanceProfile(PerformanceProfile profile) {
	this->emit_event(PROFILE_SERVICE_ON_PROFILE, {profile});
}

void EventBus::onGameEvent(CallbackWithParams&& callback) {
	this->on_with_data(STEAM_SERVICE_GAME_EVENT, std::move(callback));
}
void EventBus::emitGameEvent(size_t runningGames) {
	this->emit_event(STEAM_SERVICE_GAME_EVENT, {runningGames});
}

void EventBus::onUsbAddedRemoved(Callback&& callback) {
	this->on_without_data(HARDWARE_SERVICE_USB_ADDED_REMOVED, callback);
}
void EventBus::emitUsbAddedRemoved() {
	this->emit_event(HARDWARE_SERVICE_USB_ADDED_REMOVED);
}

void EventBus::onBattery(Callback&& callback) {
	this->on_without_data(HARDWARE_SERVICE_ON_BATTERY, callback);
}
void EventBus::emitBattery() {
	this->emit_event(HARDWARE_SERVICE_ON_BATTERY);
}