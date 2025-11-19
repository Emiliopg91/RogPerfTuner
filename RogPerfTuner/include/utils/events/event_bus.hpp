#pragma once

#include <any>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../../models/hardware/battery_charge_threshold.hpp"
#include "../../models/hardware/rgb_brightness.hpp"
#include "../../models/hardware/usb_identifier.hpp"
#include "../../models/others/singleton.hpp"
#include "../../models/performance/performance_profile.hpp"

typedef std::vector<std::any> CallbackParam;
typedef std::any CallbackAnyParam;
typedef std::function<void(CallbackParam)> CallbackWithParams;
typedef std::function<void(CallbackAnyParam)> CallbackWithAnyParam;
typedef std::function<void()> Callback;

class EventBus : public Singleton<EventBus> {
  public:
	/**
	 * Emit a notification that a new software update is available.
	 *
	 * Broadcasts an "update available" event to all listeners/subscribers registered
	 * with the event bus, carrying the provided version identifier.
	 *
	 * @param version
	 *   A string identifying the new version.
	 */
	void emitUpdateAvailable(std::string version);

	/**
	 * @brief Register a handler to be invoked when an update becomes available.
	 *
	 * Registers a callback that will be invoked each time an update event is
	 * emitted.
	 *
	 * @param callback Callable accepting a std::string; ownership of the callable
	 *                 is transferred to the event bus (moved).
	 */
	void onUpdateAvailable(std::function<void(std::string)>&& callback);

	/**
	 * @brief Emits a server socket event with the specified event name and associated value.
	 *
	 * This function triggers an event on the server socket, passing the event name and a parameter
	 * to any registered listeners or handlers.
	 *
	 * @param event The name or identifier of the event to emit.
	 * @param value The parameter or data associated with the event, encapsulated in a CallbackParam object.
	 */
	void emitServerSocketEvent(std::string event, CallbackParam value);

	/**
	 * @brief Registers a callback function to be invoked when a specific server socket event occurs.
	 *
	 * @param event The name or identifier of the server socket event to listen for.
	 * @param callback A callback function (with parameters) to be executed when the specified event is triggered.
	 */
	void onServerSocketEvent(std::string event, CallbackWithParams&& callback);

	/**
	 * @brief Emits an event over a Unix socket with the specified parameters.
	 *
	 * This function sends an event identified by its name and event type,
	 * along with an associated value, over a Unix socket. It is typically
	 * used to notify other components or processes about specific actions
	 * or state changes.
	 *
	 * @param name The name of the event source or category.
	 * @param event The specific event type or action to emit.
	 * @param value The value or data associated with the event, encapsulated in a CallbackParam.
	 */
	void emitUnixSocketEvent(std::string name, std::string event, CallbackParam value);

	/**
	 * @brief Registers a callback function to be invoked when a specific server socket event occurs.
	 *
	 * @param event The name of the server socket event to listen for.
	 * @param callback The callback function to be executed when the event is triggered.
	 *        The callback is passed as an rvalue reference to allow efficient transfer of ownership.
	 */
	void onUnixSocketEvent(std::string name, std::string event, CallbackWithParams&& callback);

	/**
	 * @brief Emits a D-Bus property event with the specified interface, property, and value.
	 *
	 * This function triggers an event corresponding to a property change on a given D-Bus interface.
	 *
	 * @param interface The name of the D-Bus interface where the property resides.
	 * @param property The name of the property that has changed.
	 * @param value The new value of the property, stored as a std::any to support multiple types.
	 */
	void emitDbusPropertyEvent(std::string interface, std::string property, std::any value);

	/**
	 * @brief Registers a callback to be invoked when a D-Bus property event occurs.
	 *
	 * This function allows you to listen for changes to a specific property on a given D-Bus interface.
	 * When the specified property changes, the provided callback will be executed with the new value.
	 *
	 * @param interface The name of the D-Bus interface to monitor.
	 * @param property The name of the property to watch for changes.
	 * @param callback A callback function that will be called with the new property value as a parameter.
	 */
	void onDbusPropertyEvent(std::string interface, std::string property, CallbackWithAnyParam&& callback);

	/**
	 * @brief Emits a D-Bus signal event with the specified interface and signal name.
	 *
	 * This function triggers a D-Bus signal event, allowing other components or services
	 * listening on the specified interface to react to the emitted signal.
	 *
	 * @param interface The D-Bus interface name on which to emit the signal.
	 * @param signal The name of the signal to emit.
	 */
	void emitDbusSignalEvent(std::string interface, std::string signal);

	/**
	 * @brief Registers a callback to be invoked when a D-Bus signal event occurs for a specific interface and property.
	 *
	 * @param interface The D-Bus interface to listen for signal events on.
	 * @param property The property of the interface to monitor for changes.
	 * @param callback The callback function to be called when the signal event is received.
	 */
	void onDbusSignalEvent(std::string interface, std::string property, Callback&& callback);

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
	 * @brief Registers a callback to be invoked when the boot sound event occurs.
	 *
	 * @param callback A callable object (such as a lambda or function) that takes a boolean parameter.
	 *                 The boolean parameter typically indicates the state or result related to the boot sound event.
	 *                 The callback is moved into the event bus and will be called when the event is triggered.
	 */
	void onBootSound(std::function<void(bool)>&& callback);

	/**
	 * @brief Emits an event to enable or disable the boot sound.
	 *
	 * This function triggers the boot sound event, allowing subscribers to respond
	 * to changes in the boot sound setting.
	 *
	 * @param enabled Set to true to enable the boot sound, or false to disable it.
	 */
	void emitBootSound(bool enabled);

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

	void emitUpdateStart();

	void onUpdateStart(Callback&& callback);

	inline const static constexpr std::string_view APPLICATION_STOP						= "APPLICATION_STOP";
	inline const static constexpr std::string_view UDEV_CLIENT_DEVICE_EVENT				= "UDEV_CLIENT_DEVICE_EVENT";
	inline const static constexpr std::string_view HARDWARE_SERVICE_USB_REMOVED			= "HARDWARE_SERVICE_USB_REMOVED";
	inline const static constexpr std::string_view HARDWARE_SERVICE_USB_ADDED			= "HARDWARE_SERVICE_USB_ADDED";
	inline const static constexpr std::string_view HARDWARE_SERVICE_ON_BATTERY			= "HARDWARE_SERVICE_ON_BATTERY";
	inline const static constexpr std::string_view HARDWARE_SERVICE_THRESHOLD_CHANGED	= "HARDWARE_SERVICE_THRESHOLD_CHANGED";
	inline const static constexpr std::string_view HARDWARE_SERVICE_BOOT_SOUND_CHANGED	= "HARDWARE_SERVICE_BOOT_SOUND_CHANGED";
	inline const static constexpr std::string_view ORGB_SERVICE_ON_BRIGHTNESS			= "ORGB_SERVICE_ON_BRIGHTNESS";
	inline const static constexpr std::string_view ORGB_SERVICE_ON_EFFECT				= "ORGB_SERVICE_ON_EFFECT";
	inline const static constexpr std::string_view ORGB_SERVICE_ON_COLOR				= "ORGB_SERVICE_ON_COLOR";
	inline const static constexpr std::string_view PROFILE_SERVICE_ON_PROFILE			= "PROFILE_SERVICE_ON_PROFILE";
	inline const static constexpr std::string_view PROFILE_SERVICE_ON_SCHEDULER			= "PROFILE_SERVICE_ON_SCHEDULER";
	inline const static constexpr std::string_view STEAM_SERVICE_GAME_EVENT				= "STEAM_SERVICE_GAME_EVENT";
	inline const static constexpr std::string_view APPLICATION_SERVICE_UPDATE_AVAILABLE = "APPLICATION_SERVICE_UPDATE_AVAILABLE";
	inline const static constexpr std::string_view APPLICATION_SERVICE_UPDATE_START		= "APPLICATION_SERVICE_UPDATE_START";

  private:
	EventBus()							 = default;
	EventBus(const EventBus&)			 = delete;
	EventBus& operator=(const EventBus&) = delete;

	std::unordered_map<std::string, std::vector<Callback>> no_params_listeners;
	std::unordered_map<std::string, std::vector<CallbackWithParams>> with_params_listeners;
	std::mutex mtx;

	void on_without_data(const std::string_view& event, Callback callback);
	void emit_event(const std::string_view& event);
	void on_with_data(const std::string_view& event, CallbackWithParams&& callback);
	void emit_event(const std::string_view& event, const std::vector<std::any>& args);

	void on_without_data(const std::string& event, Callback callback);
	void emit_event(const std::string& event);
	void on_with_data(const std::string& event, CallbackWithParams&& callback);
	void emit_event(const std::string& event, const std::vector<std::any>& args);

	friend class Singleton<EventBus>;
};
