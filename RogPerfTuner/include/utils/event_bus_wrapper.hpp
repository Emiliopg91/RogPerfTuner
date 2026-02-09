#pragma once

#include <optional>

#include "framework/abstracts/singleton.hpp"
#include "framework/events/event_bus.hpp"
#include "models/hardware/battery_charge_threshold.hpp"
#include "models/hardware/rgb_brightness.hpp"
#include "models/hardware/usb_identifier.hpp"
#include "models/performance/performance_profile.hpp"
class EventBusWrapper : public Singleton<EventBusWrapper> {
  private:
	friend class Singleton<EventBusWrapper>;
	EventBusWrapper();

	EventBus& eventBus = EventBus::getInstance();

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
};