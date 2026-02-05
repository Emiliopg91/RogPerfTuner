#pragma once

#include <libudev.h>

#include <atomic>
#include <functional>
#include <thread>
#include <tuple>
#include <vector>

#include "framework/abstracts/singleton.hpp"
#include "models/hardware/usb_identifier.hpp"
#include "utils/event_bus_wrapper.hpp"

class LsUsbClient : public Singleton<LsUsbClient> {
  private:
	struct udev* udev;
	struct udev_monitor* mon;
	std::thread runner;
	int fd;
	std::atomic<bool> stop;
	EventBusWrapper& eventBus = EventBusWrapper::getInstance();

	LsUsbClient();
	friend class Singleton<LsUsbClient>;

  public:
	~LsUsbClient();

	/**
	 * @brief Retrieves a list of USB devices, optionally filtered by a user-provided predicate.
	 *
	 * This function queries the available USB devices and returns a vector of UsbIdentifier objects
	 * representing each device. An optional filter function can be provided to select only devices
	 * that satisfy specific criteria.
	 *
	 * @param dev_filter A std::function that takes a const UsbIdentifier& and returns a bool.
	 *                   Only devices for which this function returns true will be included in the result.
	 *                   If nullptr, all devices are included.
	 * @return std::vector<UsbIdentifier> A vector containing the identifiers of the matching USB devices.
	 */
	const std::vector<UsbIdentifier> get_usb_dev(const std::function<bool(const UsbIdentifier&)>& dev_filter = nullptr);

	/**
	 * @brief Compares the currently connected USB devices with a previous list and categorizes the changes.
	 *
	 * This function compares the current state of connected USB devices against a previously recorded list.
	 * It returns a tuple containing three vectors:
	 *   - Devices that have been added since the previous state.
	 *   - Devices that have been removed since the previous state.
	 *   - Devices that remain unchanged.
	 *
	 * An optional device filter can be provided to include only specific devices in the comparison.
	 *
	 * @param previous The list of previously connected USB devices.
	 * @param dev_filter Optional filter function to select which devices to include in the comparison.
	 *                   If nullptr, all devices are considered.
	 * @return std::tuple<
	 *     std::vector<UsbIdentifier>, // Added devices
	 *     std::vector<UsbIdentifier>, // Removed devices
	 *     std::vector<UsbIdentifier>  // Unchanged devices
	 * >
	 */
	const std::tuple<std::vector<UsbIdentifier>, std::vector<UsbIdentifier>, std::vector<UsbIdentifier>> compare_connected_devs(
		const std::vector<UsbIdentifier>& previous, const std::function<bool(const UsbIdentifier&)>& dev_filter = nullptr);
};
