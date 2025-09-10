#pragma once

#include <any>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../models/hardware/battery_charge_threshold.hpp"
#include "../models/hardware/rgb_brightness.hpp"
#include "../models/performance/performance_profile.hpp"

typedef std::vector<std::any> CallbackParam;
typedef std::function<void(CallbackParam)> CallbackWithParams;
typedef std::function<void()> Callback;

class EventBus {
  private:
	EventBus()							 = default;
	EventBus(const EventBus&)			 = delete;
	EventBus& operator=(const EventBus&) = delete;

	std::unordered_map<std::string, std::vector<Callback>> no_params_listeners;
	std::unordered_map<std::string, std::vector<CallbackWithParams>> with_params_listeners;
	std::mutex mtx;

  public:
	static EventBus& getInstance() {
		static EventBus instance;
		return instance;
	}

	void on_without_data(const std::string& event, Callback callback);

	void emit_event(const std::string& event);

	void on_with_data(const std::string& event, CallbackWithParams&& callback);

	void emit_event(const std::string& event, const std::vector<std::any>& args);

	void onDeviceEvent(Callback&& callback);
	void emitDeviceEvent();

	void onApplicationStop(Callback&& callback);
	void emitApplicationStop();

	void onRgbBrightness(CallbackWithParams&& callback);
	void emitRgbBrightness(RgbBrightness brightness);

	void onChargeThreshold(CallbackWithParams&& callback);
	void emitChargeThreshold(BatteryThreshold threshold);

	void onRgbEffect(CallbackWithParams&& callback);
	void emitRgbEffect(std::string effect);

	void onPerformanceProfile(CallbackWithParams&& callback);
	void emitPerformanceProfile(PerformanceProfile profile);

	void onGameEvent(CallbackWithParams&& callback);
	void emitGameEvent(size_t runningGames);

	void onUsbAddedRemoved(Callback&& callback);
	void emitUsbAddedRemoved();

	void onBattery(Callback&& callback);
	void emitBattery();

	inline const static std::string APPLICATION_STOP				   = "APPLICATION_STOP";
	inline const static std::string UDEV_CLIENT_DEVICE_EVENT		   = "UDEV_CLIENT_DEVICE_EVENT";
	inline const static std::string HARDWARE_SERVICE_USB_ADDED_REMOVED = "HARDWARE_SERVICE_USB_ADDED_REMOVED";
	inline const static std::string HARDWARE_SERVICE_ON_BATTERY		   = "HARDWARE_SERVICE_ON_BATTERY";
	inline const static std::string HARDWARE_SERVICE_THRESHOLD_CHANGED = "HARDWARE_SERVICE_THRESHOLD_CHANGED";
	inline const static std::string ORGB_SERVICE_ON_BRIGHTNESS		   = "ORGB_SERVICE_ON_BRIGHTNESS";
	inline const static std::string ORGB_SERVICE_ON_EFFECT			   = "ORGB_SERVICE_ON_EFFECT";
	inline const static std::string PROFILE_SERVICE_ON_PROFILE		   = "PROFILE_SERVICE_ON_PROFILE";
	inline const static std::string STEAM_SERVICE_GAME_EVENT		   = "STEAM_SERVICE_GAME_EVENT";
};
