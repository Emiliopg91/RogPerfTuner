#pragma once

#include <mutex>
#include <optional>

#include "../clients/dbus/linux/upower_client.hpp"
#include "../clients/tcp/open_rgb/open_rgb_client.hpp"
#include "../models/hardware/rgb_brightness.hpp"
#include "../models/hardware/usb_identifier.hpp"
#include "../utils/configuration/configuration.hpp"
#include "../utils/events/event_bus.hpp"

class OpenRgbService : public Singleton<OpenRgbService>, Loggable {
  public:
	/**
	 * @brief Gets the device name for a given USB identifier.
	 *
	 * @param identifier The USB identifier of the device.
	 * @return The name of the device as a string.
	 */
	std::string getDeviceName(const UsbIdentifier&);

	/**
	 * @brief Gets the list of available RGB effects.
	 *
	 * @return A vector of strings representing the available effects.
	 */
	std::vector<std::string> getAvailableEffects();

	/**
	 * @brief Gets the currently active RGB effect.
	 *
	 * @return The name of the current effect as a string.
	 */
	std::string getCurrentEffect();

	/**
	 * @brief Gets the current RGB brightness.
	 *
	 * @return The current RgbBrightness value.
	 */
	RgbBrightness getCurrentBrightness();

	/**
	 * @brief Gets the current RGB color, if set.
	 *
	 * @return An optional string with the current color, or std::nullopt if not set.
	 */
	std::optional<std::string> getColor();

	/**
	 * @brief Restores the default Aura settings.
	 */
	void restoreAura();

	/**
	 * @brief Sets the RGB brightness to a new value.
	 *
	 * @param newBrightness The new brightness value to set.
	 */
	void setBrightness(const RgbBrightness& newBrightness);

	/**
	 * @brief Sets the RGB effect.
	 *
	 * @param newEffect The name of the effect to set.
	 * @param temporal If true, the effect is set temporarily.
	 */
	void setEffect(const std::string& newEffect, const bool& temporal = false);

	/**
	 * @brief Sets the RGB color.
	 *
	 * @param color The color to set as a string.
	 */
	void setColor(const std::string& color);

	/**
	 * @brief Disables a device by its USB identifier.
	 *
	 * @param identifier The USB identifier of the device to disable.
	 */
	void disableDevice(const UsbIdentifier&);

	/**
	 * @brief Increases the RGB brightness.
	 *
	 * @return The new RgbBrightness value after increasing.
	 */
	RgbBrightness increaseBrightness();

	/**
	 * @brief Decreases the RGB brightness.
	 *
	 * @return The new RgbBrightness value after decreasing.
	 */
	RgbBrightness decreaseBrightness();

	/**
	 * @brief Switches to the next available RGB effect.
	 *
	 * @return The name of the next effect as a string.
	 */
	std::string nextEffect();

  private:
	friend class Singleton<OpenRgbService>;
	OpenRgbService();

	std::mutex actionMutex;

	RgbBrightness brightness = RgbBrightness::MAX;
	std::string effect;
	std::optional<std::string> _color = std::nullopt;

	UPowerClient& uPowerClient	 = UPowerClient::getInstance();
	OpenRgbClient& openRgbClient = OpenRgbClient::getInstance();
	Configuration& configuration = Configuration::getInstance();
	EventBus& eventBus			 = EventBus::getInstance();

	void applyAura(const bool& temporal = false);
	void reload();
};