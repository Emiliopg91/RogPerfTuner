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
  public: /**
		   * @brief Registers a callback for an event without data (no parameters).
		   * @param event The event name as a string_view.
		   * @param callback The callback function to be called when the event is emitted.
		   */
	void on_without_data(const std::string_view& event, Callback callback);

	/**
	 * @brief Registers a callback for an event without data (no parameters).
	 * @param event The event name as a string.
	 * @param callback The callback function to be called when the event is emitted.
	 */
	void on_without_data(const std::string& event, Callback callback);

	/**
	 * @brief Emits an event without data (no parameters).
	 * @param event The event name as a string_view.
	 */
	void emit_event(const std::string_view& event);

	/**
	 * @brief Emits an event without data (no parameters).
	 * @param event The event name as a string.
	 */
	void emit_event(const std::string& event);

	/**
	 * @brief Registers a callback for an event with data (parameters).
	 * @param event The event name as a string_view.
	 * @param callback The callback function to be called when the event is emitted, receiving parameters.
	 */
	void on_with_data(const std::string_view& event, CallbackWithParams&& callback);

	/**
	 * @brief Registers a callback for an event with data (parameters).
	 * @param event The event name as a string.
	 * @param callback The callback function to be called when the event is emitted, receiving parameters.
	 */
	void on_with_data(const std::string& event, CallbackWithParams&& callback);

	/**
	 * @brief Emits an event with data (parameters).
	 * @param event The event name as a string_view.
	 * @param args The parameters to pass to the callback.
	 */
	void emit_event(const std::string_view& event, const std::vector<std::any>& args);

	/**
	 * @brief Emits an event with data (parameters).
	 * @param event The event name as a string.
	 * @param args The parameters to pass to the callback.
	 */
	void emit_event(const std::string& event, const std::vector<std::any>& args);

	/**
	 * @brief Registers a callback for device events.
	 * @param callback The callback function to be called when a device event occurs.
	 */
	void onDeviceEvent(Callback&& callback);

	/**
	 * @brief Emits a device event.
	 */
	void emitDeviceEvent();

	/**
	 * @brief Registers a callback for application stop events.
	 * @param callback The callback function to be called when the application stops.
	 */
	void onApplicationStop(Callback&& callback);

	/**
	 * @brief Emits an application stop event.
	 */
	void emitApplicationStop();

	/**
	 * @brief Registers a callback for RGB brightness events.
	 * @param callback The callback function to be called with the new RGB brightness.
	 */
	void onRgbBrightness(std::function<void(RgbBrightness)>&& callback);

	/**
	 * @brief Emits an RGB brightness event.
	 * @param brightness The new RGB brightness value.
	 */
	void emitRgbBrightness(const RgbBrightness& brightness);

	/**
	 * @brief Registers a callback for battery charge threshold events.
	 * @param callback The callback function to be called with the new battery threshold.
	 */
	void onChargeThreshold(std::function<void(BatteryThreshold)>&& callback);

	/**
	 * @brief Emits a battery charge threshold event.
	 * @param threshold The new battery threshold value.
	 */
	void emitChargeThreshold(const BatteryThreshold& threshold);

	/**
	 * @brief Registers a callback for RGB effect events.
	 * @param callback The callback function to be called with the new RGB effect.
	 */
	void onRgbEffect(std::function<void(std::string)>&& callback);

	/**
	 * @brief Emits an RGB effect event.
	 * @param effect The new RGB effect value.
	 */
	void emitRgbEffect(const std::string& effect);

	/**
	 * @brief Registers a callback for performance profile events.
	 * @param callback The callback function to be called with the new performance profile.
	 */
	void onPerformanceProfile(std::function<void(PerformanceProfile)>&& callback);

	/**
	 * @brief Emits a performance profile event.
	 * @param profile The new performance profile value.
	 */
	void emitPerformanceProfile(const PerformanceProfile& profile);

	/**
	 * @brief Registers a callback for game events.
	 * @param callback The callback function to be called with the number of running games.
	 */
	void onGameEvent(std::function<void(size_t)>&& callback);

	/**
	 * @brief Emits a game event.
	 * @param runningGames The number of running games.
	 */
	void emitGameEvent(const size_t& runningGames);

	/**
	 * @brief Registers a callback for USB added events.
	 * @param callback The callback function to be called when a USB device is added.
	 */
	void onUsbAdded(Callback&& callback);

	/**
	 * @brief Emits a USB added event.
	 */
	void emitUsbAdded();

	/**
	 * @brief Registers a callback for USB removed events.
	 * @param callback The callback function to be called with the list of removed USB identifiers.
	 */
	void onUsbRemoved(std::function<void(std::vector<UsbIdentifier>)>&& callback);

	/**
	 * @brief Emits a USB removed event.
	 * @param identifiers The list of removed USB identifiers.
	 */
	void emitUsbRemoved(const std::vector<UsbIdentifier>&);

	/**
	 * @brief Registers a callback for battery status events.
	 * @param callback The callback function to be called with the battery status (true if on battery).
	 */
	void onBattery(std::function<void(bool)>&& callback);

	/**
	 * @brief Emits a battery status event.
	 * @param onBattery True if running on battery, false otherwise.
	 */
	void emitBattery(const bool& onBattery);

	/**
	 * @brief Registers a callback for RGB color events.
	 * @param callback The callback function to be called with the new RGB color (optional).
	 */
	void onRgbColor(std::function<void(std::optional<std::string>)>&& callback);

	/**
	 * @brief Emits an RGB color event.
	 * @param color The new RGB color value (optional).
	 */
	void emitRgbColor(std::optional<std::string> color);

	/**
	 * @brief Registers a callback to be executed by the scheduler.
	 *
	 * This function allows you to provide a callback function that will be called by the scheduler.
	 * The callback receives an optional string parameter, which may contain additional information
	 * or be empty (std::nullopt) depending on the context in which the scheduler invokes it.
	 *
	 * @param callback A callable object (such as a lambda or function) that takes a single
	 *        std::optional<std::string> argument and returns void. The callback will be invoked
	 *        by the scheduler when appropriate.
	 */
	void onScheduler(std::function<void(std::optional<std::string>)>&& callback);

	/**
	 * @brief Emits an event related to the scheduler.
	 *
	 * This function triggers an event, optionally providing the name or identifier
	 * of a scheduler. If no scheduler is specified, the event may represent a generic
	 * or default scheduler action.
	 *
	 * @param scheduler An optional string containing the scheduler's name or identifier.
	 */
	void emitScheduler(std::optional<std::string> scheduler);

	inline const static constexpr std::string_view APPLICATION_STOP					  = "APPLICATION_STOP";
	inline const static constexpr std::string_view UDEV_CLIENT_DEVICE_EVENT			  = "UDEV_CLIENT_DEVICE_EVENT";
	inline const static constexpr std::string_view HARDWARE_SERVICE_USB_REMOVED		  = "HARDWARE_SERVICE_USB_REMOVED";
	inline const static constexpr std::string_view HARDWARE_SERVICE_USB_ADDED		  = "HARDWARE_SERVICE_USB_ADDED";
	inline const static constexpr std::string_view HARDWARE_SERVICE_ON_BATTERY		  = "HARDWARE_SERVICE_ON_BATTERY";
	inline const static constexpr std::string_view HARDWARE_SERVICE_THRESHOLD_CHANGED = "HARDWARE_SERVICE_THRESHOLD_CHANGED";
	inline const static constexpr std::string_view ORGB_SERVICE_ON_BRIGHTNESS		  = "ORGB_SERVICE_ON_BRIGHTNESS";
	inline const static constexpr std::string_view ORGB_SERVICE_ON_EFFECT			  = "ORGB_SERVICE_ON_EFFECT";
	inline const static constexpr std::string_view ORGB_SERVICE_ON_COLOR			  = "ORGB_SERVICE_ON_COLOR";
	inline const static constexpr std::string_view PROFILE_SERVICE_ON_PROFILE		  = "PROFILE_SERVICE_ON_PROFILE";
	inline const static constexpr std::string_view PROFILE_SERVICE_ON_SCHEDULER		  = "PROFILE_SERVICE_ON_SCHEDULER";
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
