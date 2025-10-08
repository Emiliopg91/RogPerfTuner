#pragma once

#include "../../../clients/shell/asusctl_client.hpp"
#include "../../../events/event_bus.hpp"
#include "../../../models/hardware/usb_identifier.hpp"
#include "../../../utils/constants.hpp"
#include "OpenRGB/Client.hpp"
#include "effects/abstract/abstract_effect.hpp"

class OpenRgbClient : public Singleton<OpenRgbClient>, Loggable {
  public:
	/**
	 * @brief Retrieves an array of devices compatible with the current client.
	 *
	 * This function queries and returns a list of devices that are recognized as compatible
	 * with the OpenRGB client. The returned array contains information about each device
	 * that can be managed or controlled through this client.
	 *
	 * @return CompatibleDeviceArray An array containing the compatible devices.
	 */
	const std::vector<UsbIdentifier> getCompatibleDevices();
	/**
	 * @brief Retrieves a list of available lighting effects supported by the device.
	 *
	 * @return A vector of strings, each representing the name of an available effect.
	 */
	const std::vector<std::string> getAvailableEffects();
	/**
	 * @brief Applies a lighting effect with specified brightness and optional color.
	 *
	 * @param effect The name of the lighting effect to apply.
	 * @param brightness The brightness settings for the effect.
	 * @param color Optional color to use for the effect, represented as a string (e.g., hex code or color name).
	 */
	void applyEffect(const std::string& effect, const RgbBrightness& brightness, const std::optional<std::string>& color);
	/**
	 * @brief Initializes the OpenRGB client.
	 *
	 * This method sets up the necessary resources and establishes any required
	 * connections for the OpenRGB client to function properly.
	 *
	 * @note Must be called before performing any operations with the client.
	 */
	void initialize();
	/**
	 * @brief Starts the OpenRGB client connection and initializes communication.
	 *
	 * This method establishes the necessary connections and prepares the client
	 * for sending and receiving data with the OpenRGB server.
	 *
	 * @note Ensure that the client is properly configured before calling this method.
	 */
	void start();
	/**
	 * @brief Stops the OpenRGB client and terminates any ongoing operations or connections.
	 *
	 * This function should be called to gracefully shut down the client, ensuring that
	 * all resources are properly released and any active TCP connections to the OpenRGB
	 * server are closed.
	 */
	void stop();
	/**
	 * @brief Disables the specified device.
	 *
	 * This function disables a device identified by the given string parameter,
	 * which typically represents the device's unique identifier or name.
	 *
	 * @param deviceId The identifier or name of the device to disable.
	 */
	void disableDevice(const std::string&);
	/**
	 * @brief Checks if the client supports color control functionality.
	 *
	 * @return true if color control is supported, false otherwise.
	 */
	bool supportsColor();
	/**
	 * @brief Retrieves the current color as a string, if available.
	 *
	 * This function attempts to obtain the current color value from the client.
	 * If a color is set or available, it returns an optional containing the color
	 * as a string (e.g., in hexadecimal or named format). If no color is set,
	 * it returns an empty optional.
	 *
	 * @return std::optional<std::string> The current color as a string, or std::nullopt if unavailable.
	 */
	const std::optional<std::string> getColor();

  private:
	friend class Singleton<OpenRgbClient>;
	std::unordered_map<std::string, std::string> compatibleDeviceNames;
	std::thread runnerThread;
	int port  = 0;
	pid_t pid = 0;
	orgb::Client client{Constants::APP_NAME};
	orgb::DeviceList detectedDevices;
	std::vector<std::unique_ptr<AbstractEffect>> availableEffects;
	std::thread udevConfigurer;
	std::vector<UsbIdentifier> compatibleDevices;

	Shell& shell				 = Shell::getInstance();
	EventBus& eventBus			 = EventBus::getInstance();
	AsusCtlClient& asusCtlClient = AsusCtlClient::getInstance();

	uint currentEffectIdx = 0;

	OpenRgbClient() : Loggable("OpenRgbClient") {
	}

	void startOpenRgbProcess();
	void startOpenRgbClient();
	void stopOpenRgbProcess();
	void getAvailableDevices();
	void configureUdev();
	void runner();
};