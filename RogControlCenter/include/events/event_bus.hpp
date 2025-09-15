#pragma once

#include <any>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../models/hardware/battery_charge_threshold.hpp"
#include "../models/hardware/rgb_brightness.hpp"
#include "../models/hardware/usb_identifier.hpp"
#include "../models/others/singleton.hpp"
#include "../models/performance/performance_profile.hpp"

typedef std::vector<std::any> CallbackParam;
typedef std::function<void(CallbackParam)> CallbackWithParams;
typedef std::function<void()> Callback;

class EventBus : public Singleton<EventBus> {
  public:
	void on_without_data(const std::string_view& event, Callback callback);
	void on_without_data(const std::string& event, Callback callback);
	void emit_event(const std::string_view& event);
	void emit_event(const std::string& event);

	void on_with_data(const std::string_view& event, CallbackWithParams&& callback);
	void on_with_data(const std::string& event, CallbackWithParams&& callback);
	void emit_event(const std::string_view& event, const std::vector<std::any>& args);
	void emit_event(const std::string& event, const std::vector<std::any>& args);

	void onDeviceEvent(Callback&& callback);
	void emitDeviceEvent();

	void onApplicationStop(Callback&& callback);
	void emitApplicationStop();

	void onRgbBrightness(std::function<void(RgbBrightness)>&& callback);
	void emitRgbBrightness(const RgbBrightness& brightness);

	void onChargeThreshold(std::function<void(BatteryThreshold)>&& callback);
	void emitChargeThreshold(const BatteryThreshold& threshold);

	void onRgbEffect(std::function<void(std::string)>&& callback);
	void emitRgbEffect(const std::string& effect);

	void onPerformanceProfile(std::function<void(PerformanceProfile)>&& callback);
	void emitPerformanceProfile(const PerformanceProfile& profile);

	void onGameEvent(std::function<void(size_t)>&& callback);
	void emitGameEvent(const size_t& runningGames);

	void onUsbAdded(Callback&& callback);
	void emitUsbAdded();

	void onUsbRemoved(std::function<void(std::vector<UsbIdentifier>)>&& callback);
	void emitUsbRemoved(const std::vector<UsbIdentifier>&);

	void onBattery(std::function<void(bool)>&& callback);
	void emitBattery(const bool& onBattery);

	inline const static constexpr std::string_view APPLICATION_STOP					  = "APPLICATION_STOP";
	inline const static constexpr std::string_view UDEV_CLIENT_DEVICE_EVENT			  = "UDEV_CLIENT_DEVICE_EVENT";
	inline const static constexpr std::string_view HARDWARE_SERVICE_USB_REMOVED		  = "HARDWARE_SERVICE_USB_REMOVED";
	inline const static constexpr std::string_view HARDWARE_SERVICE_USB_ADDED		  = "HARDWARE_SERVICE_USB_ADDED";
	inline const static constexpr std::string_view HARDWARE_SERVICE_ON_BATTERY		  = "HARDWARE_SERVICE_ON_BATTERY";
	inline const static constexpr std::string_view HARDWARE_SERVICE_THRESHOLD_CHANGED = "HARDWARE_SERVICE_THRESHOLD_CHANGED";
	inline const static constexpr std::string_view ORGB_SERVICE_ON_BRIGHTNESS		  = "ORGB_SERVICE_ON_BRIGHTNESS";
	inline const static constexpr std::string_view ORGB_SERVICE_ON_EFFECT			  = "ORGB_SERVICE_ON_EFFECT";
	inline const static constexpr std::string_view PROFILE_SERVICE_ON_PROFILE		  = "PROFILE_SERVICE_ON_PROFILE";
	inline const static constexpr std::string_view STEAM_SERVICE_GAME_EVENT			  = "STEAM_SERVICE_GAME_EVENT";

  private:
	EventBus()							 = default;
	EventBus(const EventBus&)			 = delete;
	EventBus& operator=(const EventBus&) = delete;

	std::unordered_map<std::string, std::vector<Callback>> no_params_listeners;
	std::unordered_map<std::string, std::vector<CallbackWithParams>> with_params_listeners;
	std::mutex mtx;

	friend class Singleton<EventBus>;
};
